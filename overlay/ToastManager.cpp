#include "ToastManager.h"

#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTimer>

#include <utility>

#include "Log.h"
#include "OverlayContext.h"
#include "PageManager.h"
#include "ToastContainer.h"

ToastManager::ToastManager(ToastRegistry *registry, QQmlEngine *engine,
                           PageManager *pages, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_engine(engine)
    , m_pages(pages)
{
    qCInfo(logToast) << "ToastManager created";

    //==============================
    // 宿主页面销毁 → 关掉它弹出的 Toast
    //==============================
    if (m_pages)
    {
        connect(m_pages, &PageManager::sceneDestroyed, this,
                &ToastManager::onOwnerDestroyed);
    }
}

ToastManager::~ToastManager()
{
    qCInfo(logToast) << "ToastManager destroyed";

    for (ToastEntry &entry : m_active)
        releaseEntry(entry);

    m_active.clear();

    for (ToastEntry &entry : m_waiting)
        releaseEntry(entry);

    m_waiting.clear();
}

//==============================
// 对外接口
//==============================
quint64 ToastManager::show(const QString &key, const QVariantMap &args)
{
    if (!m_registry || !m_registry->contains(key))
    {
        qCWarning(logToast) << "Toast not found:" << key;

        return 0;
    }

    const ToastInfo info = m_registry->toast(key);

    //==============================
    // 1. duplicate 策略（针对同 key 的 active + waiting）
    //==============================
    int activeIdx = indexOfKey(m_active, key);

    if (activeIdx >= 0)
    {
        switch (info.duplicate)
        {
        case ToastInfo::Collapse:
        {
            ToastEntry &existing = m_active[activeIdx];

            if (existing.context)
            {
                existing.context->setArgs(args);
                existing.context->setMessage(args.value("message").toString());
            }

            // 重置计时
            if (existing.timer)
                existing.timer->start();

            qCInfo(logToast) << "Collapse:" << key << "#" << existing.toastId;

            return existing.toastId;
        }

        case ToastInfo::Replace:
            beginClose(m_active[activeIdx].toastId);
            break;

        case ToastInfo::Reject:
            qCInfo(logToast) << "Rejected (duplicate):" << key;

            return 0;

        case ToastInfo::Parallel:
        default:
            break;
        }
    }
    else
    {
        int waitIdx = indexOfKey(m_waiting, key);

        if (waitIdx >= 0)
        {
            if (info.duplicate == ToastInfo::Reject)
                return 0;

            if (info.duplicate == ToastInfo::Collapse)
            {
                ToastEntry &existing = m_waiting[waitIdx];

                if (existing.context)
                {
                    existing.context->setArgs(args);
                    existing.context->setMessage(
                        args.value("message").toString());
                }

                return existing.toastId;
            }

            if (info.duplicate == ToastInfo::Replace)
                m_waiting.removeAt(waitIdx);
        }
    }

    //==============================
    // 2. 建条目
    //==============================
    ToastEntry entry;

    entry.toastId = m_nextToastId++;
    entry.key = info.ownerAppId + "/" + info.toastId;
    entry.info = info;
    entry.ownerInstanceId = m_pages ? m_pages->currentInstanceId() : 0;

    auto *context = new ToastContext;

    context->setToastId(info.toastId);
    context->setName(info.name);
    context->setDuration(info.duration);
    context->setMessage(args.value("message").toString());
    context->setArgs(args);

    entry.context = context;

    connect(context, &ToastContext::dismissRequested, this,
            [this, id = entry.toastId]() { beginClose(id); });

    connect(context, &OverlayContext::closed, this,
            [this, id = entry.toastId]() { finalizeClose(id); });

    connect(context, &ToastContext::actionTriggered, this,
            [this, id = entry.toastId](const QString &name,
                                       const QVariant &data) {
                emit actionTriggered(id, name, data);
            });

    //==============================
    // 3. Active 未满 → 直接显示
    //==============================
    if (m_active.size() < m_maxVisible)
    {
        if (!activate(entry))
        {
            releaseEntry(entry);

            return 0;
        }

        m_active.append(entry);

        emit countChanged();

        return entry.toastId;
    }

    //==============================
    // 4. 满位 + preempt → 顶掉严格低于自己的
    //==============================
    if (info.preempt)
    {
        int victim = findPreemptVictim(info.priority);

        if (victim >= 0)
        {
            beginClose(m_active[victim].toastId);

            // 不等旧的那条播完退场动画：位子立即让出。
            // 旧的滑出、新的滑入，视觉重叠是允许的
            if (!activate(entry))
            {
                releaseEntry(entry);

                return 0;
            }

            m_active.append(entry);

            emit countChanged();

            return entry.toastId;
        }

        qCInfo(logToast) << "Preempt, no victim:" << key
                         << OverlayPriority::toString(info.priority);
    }

    //==============================
    // 5. 溢出策略
    //==============================
    if (m_overflow == DropOldest)
    {
        int victim = findDropVictim();

        if (victim >= 0)
            beginClose(m_active[victim].toastId);

        if (!activate(entry))
        {
            releaseEntry(entry);

            return 0;
        }

        m_active.append(entry);

        emit countChanged();

        return entry.toastId;
    }

    m_waiting.append(entry);

    qCInfo(logToast) << "Queued:" << entry.key << "#" << entry.toastId
                     << "waiting:" << m_waiting.size();

    return entry.toastId;
}

void ToastManager::dismiss(quint64 toastId)
{
    beginClose(toastId);
}

void ToastManager::dismissKey(const QString &key)
{
    QList<quint64> ids;

    for (const ToastEntry &entry : std::as_const(m_active))
    {
        if (entry.key == key || entry.info.toastId == key)
            ids.append(entry.toastId);
    }

    for (const ToastEntry &entry : std::as_const(m_waiting))
    {
        if (entry.key == key || entry.info.toastId == key)
            ids.append(entry.toastId);
    }

    for (quint64 toastId : std::as_const(ids))
        beginClose(toastId);
}

void ToastManager::dismissAll()
{
    QList<quint64> ids;

    for (const ToastEntry &entry : std::as_const(m_active))
        ids.append(entry.toastId);

    for (const ToastEntry &entry : std::as_const(m_waiting))
        ids.append(entry.toastId);

    for (quint64 toastId : std::as_const(ids))
        beginClose(toastId);
}

int ToastManager::count() const
{
    return m_active.size();
}

int ToastManager::maxVisible() const
{
    return m_maxVisible;
}

void ToastManager::setMaxVisible(int max)
{
    m_maxVisible = qMax(1, max);

    pumpQueue();
}

ToastManager::Overflow ToastManager::overflow() const
{
    return m_overflow;
}

void ToastManager::setOverflow(Overflow overflow)
{
    m_overflow = overflow;
}

void ToastManager::setContainer(ToastContainer *container)
{
    m_container = container;

    if (!m_container)
        return;

    // 容器可能晚于 Toast 创建（QML 加载顺序），此处补挂已有条目
    for (const ToastEntry &entry : std::as_const(m_active))
    {
        if (entry.item)
            m_container->addItem(entry.item, entry.info.position,
                                 entry.info.priority);
    }

    m_container->relayout();
}

//==============================
// slots
//==============================
void ToastManager::onOwnerDestroyed(quint64 instanceId)
{
    QList<quint64> ids;

    for (const ToastEntry &entry : std::as_const(m_active))
    {
        if (entry.ownerInstanceId == instanceId)
            ids.append(entry.toastId);
    }

    for (const ToastEntry &entry : std::as_const(m_waiting))
    {
        if (entry.ownerInstanceId == instanceId)
            ids.append(entry.toastId);
    }

    if (ids.isEmpty())
        return;

    qCInfo(logToast) << "Owner destroyed:" << instanceId
                     << "closing toasts:" << ids.size();

    for (quint64 toastId : std::as_const(ids))
        beginClose(toastId);
}

//==============================
// 内部
//==============================
bool ToastManager::activate(ToastEntry &entry)
{
    if (!m_engine)
        return false;

    //==============================
    // 同步创建：Toast 必须立即可见
    //==============================
    // 不走 QQmlIncubator —— 异步孵化最快也要下一帧，体感是"点了没反应"。
    // Toast 结构极简，同步创建的开销可忽略；首次编译开销由 preload 消除。
    QQmlComponent *comp =
        m_registry ? m_registry->cachedComponent(entry.key) : nullptr;

    QQmlComponent *ownedComp = nullptr;

    if (!comp)
    {
        QString path = entry.info.basePath;

        path.replace(":/", "qrc:/");

        comp = new QQmlComponent(m_engine,
                                 QUrl(path + "/" + entry.info.entry), this);

        ownedComp = comp;
    }

    auto *ctx = new QQmlContext(m_engine->rootContext(), this);

    ctx->setContextProperty("ToastContext", entry.context.data());

    QObject *obj = comp->create(ctx);
    auto *item = qobject_cast<QQuickItem *>(obj);

    if (!item)
    {
        qCWarning(logToast) << "Create failed:" << entry.key << comp->errors();

        if (obj)
            obj->deleteLater();

        ctx->deleteLater();

        if (ownedComp)
            ownedComp->deleteLater();

        return false;
    }

    entry.qmlContext = ctx;
    entry.item = item;

    //==============================
    // 所有权移交：QML 根对象析构时带走 context，与 Dialog 同款
    //==============================
    if (entry.context)
        entry.context->setParent(item);

    if (m_container)
        m_container->addItem(item, entry.info.position, entry.info.priority);

    //==============================
    // 计时（duration == 0 → sticky，不建 timer）
    //==============================
    if (entry.info.duration > 0)
    {
        auto *timer = new QTimer(this);

        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this,
                [this, id = entry.toastId]() { beginClose(id); });

        timer->start(entry.info.duration);

        entry.timer = timer;
    }
    else
    {
        qCInfo(logToast) << "Sticky (duration=0):" << entry.key << "#"
                         << entry.toastId << "- only dismiss() can close it";
    }

    changeState(entry, OverlayState::Opened);

    qCInfo(logToast) << "Show:" << entry.key << "#" << entry.toastId;

    emit toastCreated(entry.toastId);

    return true;
}

void ToastManager::beginClose(quint64 toastId)
{
    int index = indexOfActive(toastId);

    if (index < 0)
    {
        // 还在排队：直接丢弃，没有 QML 对象也就没有动画
        int waitIndex = indexOfWaiting(toastId);

        if (waitIndex >= 0)
        {
            ToastEntry entry = m_waiting.takeAt(waitIndex);

            releaseEntry(entry);
        }

        return;
    }

    ToastEntry &entry = m_active[index];

    if (entry.state == OverlayState::Closing
        || entry.state == OverlayState::Destroyed)
    {
        return;
    }

    if (entry.timer)
        entry.timer->stop();

    changeState(entry, OverlayState::Closing);

    //==============================
    // 兜底：QML 若没有调 notifyClosed()，Toast 会永远卡在 Closing
    //==============================
    QTimer::singleShot(m_closeTimeoutMs, this, [this, toastId]() {
        if (indexOfActive(toastId) < 0)
            return;

        qCWarning(logToast) << "Close timeout, force finalize:" << toastId;

        finalizeClose(toastId);
    });
}

void ToastManager::finalizeClose(quint64 toastId)
{
    int index = indexOfActive(toastId);

    if (index < 0)
    {
        int waitIndex = indexOfWaiting(toastId);

        if (waitIndex >= 0)
        {
            ToastEntry entry = m_waiting.takeAt(waitIndex);

            releaseEntry(entry);
        }

        return;
    }

    ToastEntry entry = m_active.takeAt(index);

    changeState(entry, OverlayState::Destroyed);

    if (m_container && entry.item)
        m_container->removeItem(entry.item);

    releaseEntry(entry);

    qCInfo(logToast) << "Closed:" << entry.key << "#" << toastId;

    emit toastDestroyed(toastId);
    emit countChanged();

    pumpQueue();
}

void ToastManager::pumpQueue()
{
    while (!m_waiting.isEmpty() && m_active.size() < m_maxVisible)
    {
        // 优先级高的先出；同优先级 FIFO（takeAt 保序）
        int pick = 0;

        for (int i = 1; i < m_waiting.size(); ++i)
        {
            if (m_waiting[i].info.priority > m_waiting[pick].info.priority)
                pick = i;
        }

        ToastEntry entry = m_waiting.takeAt(pick);

        if (!activate(entry))
        {
            releaseEntry(entry);

            continue;
        }

        m_active.append(entry);

        emit countChanged();
    }
}

void ToastManager::releaseEntry(ToastEntry &entry)
{
    if (entry.timer)
    {
        entry.timer->stop();
        entry.timer->deleteLater();
        entry.timer = nullptr;
    }

    // item 销毁会带走挂在它下面的 context（所有权移交的结果）
    if (entry.item)
    {
        if (m_container)
            m_container->removeItem(entry.item);

        entry.item->setParentItem(nullptr);
        entry.item->deleteLater();
        entry.item = nullptr;
    }

    if (entry.qmlContext)
    {
        entry.qmlContext->deleteLater();
        entry.qmlContext = nullptr;
    }

    if (entry.context)
    {
        if (entry.context->parent())
        {
            entry.context = nullptr;
        }
        else
        {
            entry.context->deleteLater();
            entry.context = nullptr;
        }
    }
}

void ToastManager::changeState(ToastEntry &entry, OverlayState::State state)
{
    if (entry.state == state)
        return;

    entry.state = state;

    if (entry.context)
        entry.context->setState(state);
}

int ToastManager::indexOfActive(quint64 toastId) const
{
    for (int i = 0; i < m_active.size(); ++i)
    {
        if (m_active[i].toastId == toastId)
            return i;
    }

    return -1;
}

int ToastManager::indexOfWaiting(quint64 toastId) const
{
    for (int i = 0; i < m_waiting.size(); ++i)
    {
        if (m_waiting[i].toastId == toastId)
            return i;
    }

    return -1;
}

int ToastManager::indexOfKey(const QList<ToastEntry> &list,
                             const QString &key) const
{
    for (int i = 0; i < list.size(); ++i)
    {
        if (list[i].key == key || list[i].info.toastId == key)
            return i;
    }

    return -1;
}

int ToastManager::findPreemptVictim(int priority) const
{
    int victim = -1;

    for (int i = 0; i < m_active.size(); ++i)
    {
        const ToastEntry &entry = m_active[i];

        if (entry.state == OverlayState::Closing
            || entry.state == OverlayState::Destroyed)
        {
            continue;
        }

        // 只动严格低于自己的，同优先级互不干预
        if (entry.info.priority >= priority)
            continue;

        if (victim < 0
            || entry.info.priority < m_active[victim].info.priority)
        {
            victim = i;
        }
    }

    return victim;
}

int ToastManager::findDropVictim() const
{
    int victim = -1;

    for (int i = 0; i < m_active.size(); ++i)
    {
        const ToastEntry &entry = m_active[i];

        if (entry.state == OverlayState::Closing
            || entry.state == OverlayState::Destroyed)
        {
            continue;
        }

        // 严格小于才替换 → 同优先级保留更早的那条（FIFO）
        if (victim < 0
            || entry.info.priority < m_active[victim].info.priority)
        {
            victim = i;
        }
    }

    return victim;
}
