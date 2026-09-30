#include "PopupManager.h"

#include <QTimer>

#include "Log.h"
#include "OverlayState.h"
#include "PageManager.h"
#include "PopupContext.h"

PopupManager::PopupManager(PopupRegistry *registry, PageManager *pages,
                           QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_pages(pages)
{
    qCInfo(logPopup) << "PopupManager created";

    //==============================
    // 宿主页面销毁 → 关掉它弹出的弹窗
    //==============================
    // 必须早于页面 QML 根对象析构：PopupContext 已挂在根对象下，
    // 页面先死会把它带走，弹窗的退场动画就崩了
    if (m_pages)
    {
        connect(m_pages, &PageManager::sceneDestroyed, this,
                &PopupManager::onOwnerDestroyed);
    }
}

PopupManager::~PopupManager()
{
    qCInfo(logPopup) << "PopupManager destroyed";

    // 退出阶段事件循环即将停止，deleteLater 不会被执行，此处同步释放。
    // context 有 parent 说明已随 QML 根对象销毁，无需处理。
    for (const PopupInstance &instance : m_stack)
    {
        if (instance.context && !instance.context->parent())
            delete instance.context.data();
    }

    m_stack.clear();
    m_results.clear();
}

//==============================
// open / close
//==============================
quint64 PopupManager::open(const QString &key, const QVariantMap &args)
{
    if (!m_registry || !m_registry->contains(key))
    {
        qCWarning(logPopup) << "Popup not found:" << key;

        return 0;
    }

    PopupInfo info = m_registry->popup(key);

    //==============================
    // 1. 准入：Reject 且已有优先级 >= 自己的 → 放弃显示
    //==============================
    if (info.conflict == PopupInfo::Reject && !admit(info))
    {
        qCInfo(logPopup) << "Rejected (lower priority):" << key
                         << OverlayPriority::toString(info.priority)
                         << "<=" << OverlayPriority::toString(topPriority());

        emit popupRejected(key, info.priority);

        return 0;
    }

    //==============================
    // 2. 抢占：关闭所有优先级严格低于自己的
    //==============================
    // 同优先级永不互相干预
    if (info.conflict == PopupInfo::Preempt)
    {
        QList<quint64> victims;

        for (const PopupInstance &other : m_stack)
        {
            if (other.info.priority < info.priority
                && other.state != OverlayState::Closing)
            {
                victims.append(other.popupId);
            }
        }

        for (quint64 victim : victims)
            preemptClose(victim);
    }

    //==============================
    // 3. 入栈
    //==============================
    PopupInstance instance = createInstance(info, args);

    m_stack.append(instance);

    changeState(m_stack.last(), OverlayState::Creating);

    qCInfo(logPopup) << "Open" << instance.key << "#" << instance.popupId
                     << "priority:" << OverlayPriority::toString(info.priority);

    emit popupCreated(m_stack.last());
    emit countChanged();

    updateCoveredState();

    return instance.popupId;
}

void PopupManager::close(quint64 popupId, const QVariant &result)
{
    beginClose(popupId, result);
}

void PopupManager::closeTop(const QVariant &result)
{
    // 退场动画播放期间不再响应，避免连按关闭掉两层
    if (hasClosing())
        return;

    quint64 popupId = visualTopId();

    if (!popupId)
        return;

    PopupInstance *instance = findInstance(popupId);

    // 强制类弹窗（closeOnBackKey=false）不响应返回键
    if (instance && !instance->info.closeOnBackKey)
    {
        qCInfo(logPopup) << "Back key ignored (closeOnBackKey=false):"
                         << popupId;

        return;
    }

    close(popupId, result);
}

void PopupManager::closeAll()
{
    QList<quint64> ids;

    for (const PopupInstance &instance : m_stack)
        ids.append(instance.popupId);

    for (quint64 popupId : ids)
        beginClose(popupId, QVariant());
}

//==============================
// 查询
//==============================
int PopupManager::count() const
{
    return m_stack.size();
}

bool PopupManager::hasModal() const
{
    for (const PopupInstance &instance : m_stack)
    {
        if (instance.info.kind == PopupInfo::Modal
            && instance.state != OverlayState::Closing)
        {
            return true;
        }
    }

    return false;
}

int PopupManager::topPriority() const
{
    int top = -1;

    for (const PopupInstance &instance : m_stack)
        top = qMax(top, instance.info.priority);

    return top;
}

//==============================
// slots
//==============================
void PopupManager::popupReady(quint64 popupId)
{
    PopupInstance *instance = findInstance(popupId);

    if (!instance)
        return;

    qCInfo(logPopup) << "Ready:" << instance->key << "#" << popupId;

    changeState(*instance, OverlayState::Opened);
}

void PopupManager::onOwnerDestroyed(quint64 instanceId)
{
    QList<quint64> ids;

    for (const PopupInstance &instance : m_stack)
    {
        if (instance.ownerInstanceId == instanceId)
            ids.append(instance.popupId);
    }

    if (ids.isEmpty())
        return;

    qCInfo(logPopup) << "Owner destroyed:" << instanceId
                     << "closing popups:" << ids.size();

    for (quint64 popupId : ids)
        beginClose(popupId, QVariant());
}

//==============================
// 内部
//==============================
PopupInstance PopupManager::createInstance(const PopupInfo &info,
                                           const QVariantMap &args)
{
    PopupInstance instance;

    instance.popupId = m_nextPopupId++;
    instance.key = info.ownerAppId + "/" + info.popupId;
    instance.info = info;
    instance.args = args;
    instance.ownerInstanceId = m_pages ? m_pages->currentInstanceId() : 0;

    auto *context = new PopupContext;

    context->setPopupId(info.popupId);
    context->setName(info.name);
    context->setModal(info.kind == PopupInfo::Modal);
    context->setDim(info.dim);
    context->setArgs(args);

    instance.context = context;

    // QML 请求关闭 → 置 Closing，等退场动画
    connect(context, &PopupContext::closeRequested, this,
            [this, popupId = instance.popupId](const QVariant &result) {
                beginClose(popupId, result);
            });

    // 退场动画播完 → 真正销毁
    connect(context, &OverlayContext::closed, this,
            [this, popupId = instance.popupId]() { finalizeClose(popupId); });

    return instance;
}

void PopupManager::changeState(PopupInstance &instance, OverlayState::State state)
{
    if (instance.state == state)
        return;

    instance.state = state;

    if (instance.context)
        instance.context->setState(state);
}

bool PopupManager::admit(const PopupInfo &info) const
{
    // 已有优先级 >= 自己的弹窗在显示 → 不允许弹
    for (const PopupInstance &other : m_stack)
    {
        if (other.state == OverlayState::Closing)
            continue;

        if (other.info.priority >= info.priority)
            return false;
    }

    return true;
}

void PopupManager::preemptClose(quint64 popupId)
{
    // 走正常关闭流程（置 Closing → QML 播退场动画 → notifyClosed），
    // 所以 popupResult 照常发出，调用方的 callback 不会泄漏。
    beginClose(popupId, QVariant());

    // 额外通知：让调用方能区分"被顶掉"与"用户取消"
    emit popupPreempted(popupId);
}

void PopupManager::beginClose(quint64 popupId, const QVariant &result)
{
    PopupInstance *instance = findInstance(popupId);

    if (!instance)
        return;

    if (instance->state == OverlayState::Closing
        || instance->state == OverlayState::Destroyed)
    {
        return;
    }

    qCInfo(logPopup) << "Closing:" << instance->key << "#" << popupId;

    m_results.insert(popupId, result);

    if (instance->context)
        instance->context->setResult(result);

    changeState(*instance, OverlayState::Closing);

    //==============================
    // 兜底：QML 若没有调 notifyClosed()，弹窗会永远卡在 Closing
    //==============================
    QTimer::singleShot(m_closeTimeoutMs, this, [this, popupId]() {
        PopupInstance *still = findInstance(popupId);

        if (still && still->state == OverlayState::Closing)
        {
            qCWarning(logPopup) << "Close timeout, force finalize:" << popupId;

            finalizeClose(popupId);
        }
    });
}

void PopupManager::finalizeClose(quint64 popupId)
{
    int index = indexOf(popupId);

    if (index < 0)
        return;

    PopupInstance instance = m_stack.takeAt(index);

    QVariant result = m_results.take(popupId);

    changeState(instance, OverlayState::Destroyed);

    // Container 收到后移除 view（view 的 rootItem 销毁会带走 context）
    emit popupDestroyed(popupId);

    releaseContext(instance);

    qCInfo(logPopup) << "Closed:" << instance.key << "#" << popupId;

    emit popupResult(popupId, result);
    emit countChanged();

    updateCoveredState();
}

quint64 PopupManager::visualTopId() const
{
    quint64 topId = 0;
    int topPriority = -1;

    // 与 PopupContainer::restack() 同一套排序：
    // 优先级高者在上，同优先级后入栈者在上
    for (const PopupInstance &instance : m_stack)
    {
        if (instance.state == OverlayState::Closing
            || instance.state == OverlayState::Destroyed)
        {
            continue;
        }

        if (!topId || instance.info.priority >= topPriority)
        {
            topPriority = instance.info.priority;
            topId = instance.popupId;
        }
    }

    return topId;
}

bool PopupManager::hasClosing() const
{
    for (const PopupInstance &instance : m_stack)
    {
        if (instance.state == OverlayState::Closing)
            return true;
    }

    return false;
}

void PopupManager::updateCoveredState()
{
    // 只有模态弹窗才覆盖下层；非模态不改变页面状态
    const bool covered = hasModal();

    if (covered == m_covered)
        return;

    m_covered = covered;

    if (m_pages)
        m_pages->setTopPageCovered(covered);
}

void PopupManager::releaseContext(PopupInstance &instance)
{
    if (!instance.context)
        return;

    // 已有 parent 说明所有权已移交给 QML 根对象，随其销毁即可
    if (instance.context->parent())
    {
        instance.context = nullptr;
        return;
    }

    // QML 从未创建成功，没有对象树接管，此处兜底
    instance.context->deleteLater();
    instance.context = nullptr;
}

int PopupManager::indexOf(quint64 popupId) const
{
    for (int i = 0; i < m_stack.size(); ++i)
    {
        if (m_stack[i].popupId == popupId)
            return i;
    }

    return -1;
}

PopupInstance *PopupManager::findInstance(quint64 popupId)
{
    for (PopupInstance &instance : m_stack)
    {
        if (instance.popupId == popupId)
            return &instance;
    }

    return nullptr;
}
