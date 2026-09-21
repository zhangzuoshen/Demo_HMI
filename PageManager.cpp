#include "PageManager.h"

#include "AppContext.h"
#include "Log.h"

PageManager::PageManager(AppRegistry *registry,
                         QObject *parent)
    : QObject(parent)
    , m_registry(registry)
{
    qCInfo(logPageManager) << "PageManager created";
}

QString PageManager::currentAppId() const
{
    return m_stack.isEmpty() ? QString() : m_stack.last().info.appId;
}

quint64 PageManager::currentInstanceId() const
{
    return m_stack.isEmpty() ? 0 : m_stack.last().instanceId;
}

AppInstance PageManager::currentApp() const
{
    return m_stack.isEmpty() ? AppInstance() : m_stack.last();
}

const QVector<AppInstance> &PageManager::stack() const
{
    return m_stack;
}

AppInstance PageManager::createInstance(const AppInfo &info)
{
    AppInstance instance;

    instance.info = info;
    instance.instanceId = m_nextInstanceId++;
    instance.state = AppState::Created;

    instance.context = new AppContext();

    instance.context->setAppId(info.appId);
    instance.context->setAppName(info.name);
    instance.context->setInstanceId(instance.instanceId);
    instance.context->setState(AppState::Created);

    return instance;
}

void PageManager::setState(AppInstance &instance,
                           AppState::State state)
{
    if (instance.state == state)
        return;

    instance.state = state;

    if (instance.context)
        instance.context->setState(state);

    qCInfo(logPageManager)
            << "["
            << instance.instanceId
            << "]"
            << instance.info.appId
            << "State ->"
            << AppState::toString(state);
}

void PageManager::dispatchLifecycle(AppInstance &instance,
                                    AppState::State state)
{
    switch (state)
    {
    case AppState::Ready:
        emit appReady(instance.instanceId);
        break;

    case AppState::Foreground:
        emit appEntered(instance.instanceId);
        break;

    case AppState::Background:
        emit appPaused(instance.instanceId);
        break;

    case AppState::Destroyed:
        emit appExited(instance.instanceId);
        break;

    default:
        break;
    }
}

void PageManager::pageReady(quint64 instanceId)
{
    qCInfo(logPageManager)
            << "pageReady:"
            << instanceId;

    AppInstance *instance = findInstance(instanceId);

    if (!instance)
        return;

    if (instance->state != AppState::Created)
    {
        qCDebug(logPageManager)
                << "Already Ready";

        return;
    }

    setState(*instance,
             AppState::Ready);

    dispatchLifecycle(*instance,
                      AppState::Ready);

    setState(*instance,
             AppState::Foreground);

    dispatchLifecycle(*instance,
                      AppState::Foreground);
}

void PageManager::pageAttached(quint64 instanceId)
{
    AppInstance *instance = findInstance(instanceId);

    if (!instance)
        return;

    if (instance->state == AppState::Foreground)
    {
        qCInfo(logPageManager)
                << "["
                << instanceId
                << "]"
                << instance->info.appId
                << "Resume";

        emit appResumed(instanceId);
    }
}

void PageManager::launchApp(const QString &appId,
                            LaunchReason)
{
    qCInfo(logPageManager)
            << "Launch request:"
            << appId;

    AppInfo target = m_registry->app(appId);

    switch (target.launchMode)
    {
    case AppInfo::SingleTask:
    {
        int index = findTask(appId);

        // 已存在前台任务
        if (index >= 0)
        {
            while (m_stack.size() - 1 > index)
            {
                AppInstance dead = m_stack.takeLast();

                if (dead.info.keepAlive)
                {
                    qCInfo(logPageManager)
                            << "["
                            << dead.instanceId
                            << "]"
                            << "\"" + dead.info.appId + "\""
                            << "Move to Background Cache";

                    setState(dead,
                             AppState::Background);

                    dispatchLifecycle(dead,
                                      AppState::Background);

                    m_backgroundApps.insert(dead.info.appId,
                                            dead);

                    emit sceneDetached(dead.instanceId);
                }
                else
                {
                    setState(dead,
                             AppState::Destroyed);

                    dispatchLifecycle(dead,
                                      AppState::Destroyed);

                    emit sceneDestroyed(dead.instanceId);

                    dead.context->deleteLater();

                    emit instanceDestroyed(dead.instanceId,
                                           dead.info.appId);
                }
            }

            setState(m_stack.last(),
                     AppState::Foreground);

            emit sceneAttached(m_stack.last().instanceId);

            emit currentChanged();

            return;
        }

        // 后台恢复
        if (m_backgroundApps.contains(appId))
        {
            if (!m_stack.isEmpty())
            {
                setState(m_stack.last(),
                         AppState::Background);

                dispatchLifecycle(m_stack.last(),
                                  AppState::Background);
            }

            AppInstance instance =
                    m_backgroundApps.take(appId);

            setState(instance,
                     AppState::Foreground);

            m_stack.append(instance);

            qCInfo(logPageManager)
                    << "["
                    << instance.instanceId
                    << "]"
                    << "\"" + appId + "\""
                    << "Restore from Background Cache";

            emit sceneAttached(instance.instanceId);

            emit currentChanged();

            return;
        }

        // 首次创建
        if (!m_stack.isEmpty())
        {
            setState(m_stack.last(),
                     AppState::Background);

            dispatchLifecycle(m_stack.last(),
                              AppState::Background);
        }

        m_stack.append(createInstance(target));

        emit instanceCreated(m_stack.last().instanceId,
                             appId);

        emit sceneCreated(m_stack.last());

        emit currentChanged();

        break;
    }

    case AppInfo::Standard:
    default:
    {
        if (!m_stack.isEmpty())
        {
            setState(m_stack.last(),
                     AppState::Background);

            dispatchLifecycle(m_stack.last(),
                              AppState::Background);
        }

        m_stack.append(createInstance(target));

        emit instanceCreated(m_stack.last().instanceId,
                             appId);

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

    AppInstance current = m_stack.takeLast();

    if (current.info.keepAlive)
    {
        qCInfo(logPageManager)
                << "["
                << current.instanceId
                << "]"
                << "\"" + current.info.appId + "\""
                << "Move to Background Cache";

        setState(current,
                 AppState::Background);

        dispatchLifecycle(current,
                          AppState::Background);

        m_backgroundApps.insert(current.info.appId,
                                current);

        emit sceneDetached(current.instanceId);
    }
    else
    {
        setState(current,
                 AppState::Destroyed);

        dispatchLifecycle(current,
                          AppState::Destroyed);

        emit sceneDestroyed(current.instanceId);

        current.context->deleteLater();

        emit instanceDestroyed(current.instanceId,
                               current.info.appId);
    }

    AppInstance &prev = m_stack.last();

    setState(prev,
             AppState::Foreground);

    emit sceneAttached(prev.instanceId);

    emit currentChanged();
}

AppInstance *PageManager::findInstance(quint64 instanceId)
{
    for (auto &instance : m_stack)
    {
        if (instance.instanceId == instanceId)
            return &instance;
    }

    return nullptr;
}

const AppInstance *PageManager::findInstance(quint64 instanceId) const
{
    for (const auto &instance : m_stack)
    {
        if (instance.instanceId == instanceId)
            return &instance;
    }

    return nullptr;
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
