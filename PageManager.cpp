#include "PageManager.h"

#include "Log.h"

PageManager::PageManager(AppRegistry *registry, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
{
    qCInfo(logPageManager) << "PageManager created";
}

PageManager::~PageManager()
{
    qCInfo(logPageManager) << "PageManager destroyed";

    // 退出阶段事件循环即将停止，deleteLater 不会被执行，此处同步释放。
    // context 为 null 说明已随 QML 根对象销毁，无需处理。
    for (const AppInstance &instance : m_stack)
    {
        if (instance.context && !instance.context->parent())
            delete instance.context.data();
    }

    for (const AppInstance &instance : m_backgroundCache)
    {
        if (instance.context && !instance.context->parent())
            delete instance.context.data();
    }

    m_stack.clear();
    m_backgroundCache.clear();
}

QString PageManager::currentAppId() const
{
    if (m_stack.isEmpty())
        return QString();

    return m_stack.last().info.appId;
}

quint64 PageManager::currentInstanceId() const
{
    if (m_stack.isEmpty())
        return 0;

    return m_stack.last().instanceId;
}

bool PageManager::canGoBack() const
{
    return m_stack.size() > 1;
}

void PageManager::launch(const QString &appId, const QVariantMap &args)
{
    qCInfo(logPageManager) << "Launch request:" << appId;

    if (!m_registry || !m_registry->contains(appId))
    {
        qCWarning(logPageManager) << "App not found:" << appId;
        return;
    }

    AppInfo target = m_registry->app(appId);

    switch (target.launchMode)
    {
    case AppInfo::SingleTask:
    {
        //--------------------------------------------------
        // 已在前台栈
        //--------------------------------------------------
        int index = findTask(appId);

        if (index >= 0)
        {
            while (m_stack.size() - 1 > index)
            {
                AppInstance dead = m_stack.takeLast();

                if (dead.info.keepAlive)
                {
                    qCInfo(logPageManager) << "[" << dead.instanceId << "]"
                                           << "\"" << dead.info.appId << "\""
                                           << "Move to Background Cache";

                    changeState(dead, AppState::Background);
                    m_backgroundCache.append(dead);
                    emit sceneDetached(dead.instanceId);

                    continue; // 保留，不执行下面销毁代码
                }

                // keepAlive=false：销毁
                changeState(dead, AppState::Destroyed);
                emit sceneDestroyed(dead.instanceId);
                releaseContext(dead);
            }

            AppInstance &top = m_stack.last();

            top.context->setLaunchArgs(args);
            changeState(top, AppState::Foreground);
            emit sceneAttached(top.instanceId);
            emit currentChanged();

            return;
        }

        //--------------------------------------------------
        // 后台缓存恢复
        //--------------------------------------------------
        int bg = findBackground(appId);

        if (bg >= 0)
        {
            if (!m_stack.isEmpty())
                changeState(m_stack.last(), AppState::Background);

            AppInstance instance = m_backgroundCache.takeAt(bg);

            m_stack.append(instance);

            AppInstance &top = m_stack.last();
            top.context->setLaunchArgs(args);
            changeState(top, AppState::Foreground);
            emit sceneAttached(instance.instanceId);
            emit currentChanged();

            return;
        }

        //--------------------------------------------------
        // 新建实例
        //--------------------------------------------------
        if (!m_stack.isEmpty())
            changeState(m_stack.last(), AppState::Background);

        m_stack.append(createInstance(target, args));
        emit sceneCreated(m_stack.last());
        emit currentChanged();

        break;
    }

    case AppInfo::SingleTop:
    {
        if (!m_stack.isEmpty() && m_stack.last().info.appId == appId)
        {
            qCInfo(logPageManager) << "SingleTop ignored:" << appId;
            return;
        }

        if (!m_stack.isEmpty())
            changeState(m_stack.last(), AppState::Background);

        m_stack.append(createInstance(target, args));
        emit sceneCreated(m_stack.last());
        emit currentChanged();

        break;
    }

    case AppInfo::Standard:
    default:
    {
        if (!m_stack.isEmpty())
            changeState(m_stack.last(), AppState::Background);

        m_stack.append(createInstance(target, args));
        emit sceneCreated(m_stack.last());
        emit currentChanged();

        break;
    }
    }
}

void PageManager::back()
{
    if (m_stack.size() <= 1)
        return;

    AppInstance dead = m_stack.takeLast();
    changeState(dead, AppState::Destroyed);
    emit sceneDestroyed(dead.instanceId);
    releaseContext(dead);

    AppInstance &next = m_stack.last();

    changeState(next, AppState::Foreground);
    emit sceneAttached(next.instanceId);
    emit currentChanged();
}

void PageManager::setTopPageCovered(bool covered)
{
    if (m_stack.isEmpty())
        return;

    AppInstance &top = m_stack.last();

    if (covered)
    {
        if (top.state == AppState::Foreground)
            changeState(top, AppState::Covered);
    }
    else
    {
        if (top.state == AppState::Covered)
            changeState(top, AppState::Foreground);
    }
}

void PageManager::pageReady(quint64 instanceId)
{
    qCInfo(logPageManager) << "pageReady:" << instanceId;

    AppInstance *instance = findInstance(instanceId);

    if (!instance)
        return;

    if (instance->state != AppState::Created)
    {
        qCDebug(logPageManager) << "Already Ready";
        return;
    }

    changeState(*instance, AppState::Ready);

    if (instance == &m_stack.last())
        changeState(*instance, AppState::Foreground);
}

AppInstance PageManager::createInstance(const AppInfo &info,
                                        const QVariantMap &args)
{
    AppInstance instance;

    instance.instanceId = m_nextInstanceId++;

    instance.info = info;
    instance.state = AppState::Created;

    instance.context = new AppContext;
    instance.context->setAppId(info.appId);
    instance.context->setAppName(info.name);
    instance.context->setInstanceId(instance.instanceId);
    instance.context->setLaunchArgs(args);
    instance.context->setState(AppState::Created);
    instance.context->setWindowState(WindowState::Creating);

    return instance;
}

void PageManager::changeState(AppInstance &instance, AppState::State state)
{
    if (instance.state == state)
        return;

    instance.state = state;

    if (instance.context)
    {
        if (state != AppState::Ready && state != AppState::Foreground)
        {
            instance.context->clearLaunchArgs();
        }

        instance.context->setState(state);
    }

    qCInfo(logPageManager) << "[" << instance.instanceId << "]"
                           << "\"" << instance.info.appId << "\""
                           << "State ->" << AppState::toString(state);
}

int PageManager::findTask(const QString &appId) const
{
    for (int i = 0; i < m_stack.size(); ++i)
    {
        if (m_stack[i].info.appId == appId)
            return i;
    }

    return -1;
}

int PageManager::findBackground(const QString &appId) const
{
    for (int i = 0; i < m_backgroundCache.size(); ++i)
    {
        if (m_backgroundCache[i].info.appId == appId)
            return i;
    }

    return -1;
}

void PageManager::releaseContext(AppInstance &instance)
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

AppInstance *PageManager::findInstance(quint64 instanceId)
{
    for (AppInstance &instance : m_stack)
    {
        if (instance.instanceId == instanceId)
            return &instance;
    }

    for (AppInstance &instance : m_backgroundCache)
    {
        if (instance.instanceId == instanceId)
            return &instance;
    }

    return nullptr;
}
