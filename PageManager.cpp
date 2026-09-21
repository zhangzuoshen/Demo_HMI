#include "PageManager.h"

#include "AppContext.h"
#include "Log.h"

PageManager::PageManager(
        AppRegistry *registry,
        QObject *parent)
    : QObject(parent),
      m_registry(registry)
{
    qCInfo(logPageManager)
            << "PageManager created";
}

QString PageManager::currentAppId() const
{
    return m_stack.isEmpty()
            ? QString()
            : m_stack.last().info.appId;
}

quint64 PageManager::currentInstanceId() const
{
    return m_stack.isEmpty()
            ? 0
            : m_stack.last().instanceId;
}

AppInstance PageManager::currentApp() const
{
    return m_stack.isEmpty()
            ? AppInstance()
            : m_stack.last();
}

const QVector<AppInstance> &PageManager::stack() const
{
    return m_stack;
}

AppInstance PageManager::createInstance(const AppInfo &info)
{
    AppInstance instance;

    instance.instanceId = m_nextInstanceId++;
    instance.info = info;
    instance.firstLaunch = true;
    instance.waitingForReady = true;
    instance.state = AppState::Created;

    instance.context = new AppContext();
    instance.context->initialize(instance);

    return instance;
}

void PageManager::changeState(
        AppInstance &instance,
        AppState::State state)
{
    if(instance.state==state)
        return;

    instance.state=state;

    qCInfo(logPageManager)
            << "["
            << instance.instanceId
            << "]"
            << instance.info.appId
            << "State ->"
            << state;

    if(instance.context)
        instance.context->setState(state);
}

AppInstance *PageManager::findInstance(
        quint64 instanceId)
{
    for(auto &instance:m_stack)
    {
        if(instance.instanceId==instanceId)
            return &instance;
    }

    return nullptr;
}

const AppInstance *PageManager::findInstance(
        quint64 instanceId) const
{
    for(const auto &instance:m_stack)
    {
        if(instance.instanceId==instanceId)
            return &instance;
    }

    return nullptr;
}

int PageManager::findTask(
        const QString &appId) const
{
    for(int i=0;i<m_stack.size();i++)
    {
        if(m_stack[i].info.appId==appId)
            return i;
    }

    return -1;
}

void PageManager::launchApp(
        const QString &appId,
        LaunchReason reason)
{
    if(!m_registry->contains(appId))
        return;

    AppInfo target=
            m_registry->app(appId);

    if(reason==System &&
       !m_stack.isEmpty() &&
       target.priority<m_stack.last().info.priority)
        return;

    switch(target.launchMode)
    {

    case AppInfo::Standard:
    {
        if (!m_stack.isEmpty())
            changeState(m_stack.last(), AppState::Background);

        m_stack.append(createInstance(target));

        emit instanceCreated(
                m_stack.last().instanceId,
                appId);

        emit sceneCreated(m_stack.last());

        emit currentChanged();

        break;
    }
    case AppInfo::SingleTop:
    {
        if(!m_stack.isEmpty() &&
           m_stack.last().info.appId==appId)
        {
            qCInfo(logPageManager)
                    << "SingleTop reuse";

            return;
        }

        if(!m_stack.isEmpty())
            changeState(
                        m_stack.last(),
                        AppState::Background);

        m_stack.append(
                    createInstance(target));

        emit instanceCreated(
                    m_stack.last().instanceId,
                    appId);

        emit sceneCreated(m_stack.last());

        emit currentChanged();

        break;
    }
    case AppInfo::SingleTask:
    {
        int index = findTask(appId);

        // ---------- 已存在实例 ----------
        if (index >= 0)
        {
            while (m_stack.size() - 1 > index)
            {
                AppInstance dead = m_stack.takeLast();

                if (dead.info.keepAlive)
                {
                    qCInfo(logPageManager)
                            << "[" << dead.instanceId << "]"
                            << dead.info.appId
                            << "Move to Background Cache";

                    changeState(dead, AppState::Background);

                    m_backgroundApps.insert(dead.info.appId, dead);

                    emit sceneDetached(dead.instanceId);
                }
                else
                {
                    qCInfo(logPageManager)
                            << "[" << dead.instanceId << "]"
                            << dead.info.appId
                            << "Destroyed";

                    changeState(dead, AppState::Destroyed);

                    emit sceneDestroyed(dead.instanceId);

                    delete dead.context;

                    emit instanceDestroyed(
                                dead.instanceId,
                                dead.info.appId);
                }
            }

            changeState(m_stack.last(),
                        AppState::Foreground);

            // 注意：这里不是 Create，而是 Attach
            emit sceneAttached(m_stack.last().instanceId);

            emit currentChanged();

            return;
        }

        // ---------- 后台缓存恢复 ----------
        if (m_backgroundApps.contains(appId))
        {
            if (!m_stack.isEmpty())
                changeState(m_stack.last(),
                            AppState::Background);

            AppInstance instance =
                    m_backgroundApps.take(appId);

            changeState(instance,
                        AppState::Foreground);

            m_stack.append(instance);

            qCInfo(logPageManager)
                    << "[" << instance.instanceId << "]"
                    << appId
                    << "Restore from Background Cache";

            emit sceneAttached(instance.instanceId);

            emit currentChanged();

            return;
        }

        // ---------- 首次创建 ----------
        if (!m_stack.isEmpty())
            changeState(m_stack.last(),
                        AppState::Background);

        m_stack.append(createInstance(target));

        emit instanceCreated(
                    m_stack.last().instanceId,
                    appId);

        emit sceneCreated(m_stack.last());

        emit currentChanged();

        break;
    }

    }

}

void PageManager::pageReady(
        quint64 instanceId)
{
    qCInfo(logPageManager)
            << "pageReady:"
            << instanceId;

    AppInstance *instance=
            findInstance(instanceId);

    if(!instance)
    {
        qCWarning(logPageManager)
                << "Instance not found";

        return;
    }

    if(!instance->waitingForReady)
    {
        qCDebug(logPageManager)
                << "Already Ready";

        return;
    }

    instance->waitingForReady=false;

    changeState(
                *instance,
                AppState::Ready);

    changeState(
                *instance,
                AppState::Foreground);

    instance->firstLaunch=false;

    if(instance->context)
        instance->context->setFirstLaunch(false);
}

void PageManager::back()
{
    if (m_stack.size() <= 1)
        return;

    AppInstance current = m_stack.takeLast();

    if (current.info.keepAlive)
    {
        qCInfo(logPageManager)
                << "[" << current.instanceId << "]"
                << current.info.appId
                << "Move to Background Cache";

        changeState(current, AppState::Background);

        m_backgroundApps.insert(current.info.appId, current);

        emit sceneDetached(current.instanceId);
    }
    else
    {
        qCInfo(logPageManager)
                << "[" << current.instanceId << "]"
                << current.info.appId
                << "Destroyed";

        changeState(current, AppState::Destroyed);

        emit sceneDestroyed(current.instanceId);

        delete current.context;
    }

    AppInstance &prev = m_stack.last();

    changeState(prev, AppState::Foreground);

    emit sceneAttached(prev.instanceId);

    emit currentChanged();
}
