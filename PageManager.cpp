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

AppInstance PageManager::createInstance(
        const AppInfo &info)
{
    AppInstance instance;

    instance.instanceId=m_nextInstanceId++;

    instance.info=info;

    instance.firstLaunch=true;

    instance.waitingForReady=true;

    instance.state=AppState::Created;

    instance.context=new AppContext();

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

    if(instance.context)
        instance.context->setState(state);

    qCInfo(logPageManager)
            << "["
            << instance.instanceId
            << "]"
            << instance.info.appId
            << "State ->"
            << state;
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
        if(!m_stack.isEmpty())
            changeState(
                        m_stack.last(),
                        AppState::Background);

        m_stack.append(
                    createInstance(target));

        emit instanceCreated(
                    m_stack.last().instanceId,
                    appId);

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

        emit currentChanged();

        break;
    }
    case AppInfo::SingleTask:
    {
        int index=
                findTask(appId);

        if(index>=0)
        {
            while(m_stack.size()-1>index)
            {
                AppInstance dead=
                        m_stack.takeLast();

                changeState(
                            dead,
                            AppState::Destroyed);

                delete dead.context;

                emit instanceDestroyed(
                            dead.instanceId,
                            dead.info.appId);
            }

            changeState(
                        m_stack.last(),
                        AppState::Foreground);

            emit currentChanged();

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
    if(m_stack.size()<=1)
        return;

    AppInstance dead=
            m_stack.takeLast();

    changeState(
                dead,
                AppState::Destroyed);

    delete dead.context;

    emit instanceDestroyed(
                dead.instanceId,
                dead.info.appId);

    changeState(
                m_stack.last(),
                AppState::Foreground);

    emit currentChanged();
}
