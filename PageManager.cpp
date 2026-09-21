#include "PageManager.h"

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

const QVector<AppInstance>& PageManager::stack() const
{
    return m_stack;
}

AppInstance PageManager::createInstance(
        const AppInfo &info)
{
    AppInstance instance;

    instance.instanceId=m_nextInstanceId++;

    instance.firstLaunch=true;

    instance.info=info;

    instance.state=AppState::Created;

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
            << "["<<instance.instanceId<<"]"
            << instance.info.appId
            << "State ->"
            << state;

    emit instanceStateChanged(
                instance.instanceId,
                static_cast<int>(state));
}

int PageManager::findTask(
        const QString &appId) const
{
    for(int i=0;i<m_stack.size();++i)
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

    AppInfo target=m_registry->app(appId);

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

        m_pendingReadyId=
                m_stack.last().instanceId;

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

        m_pendingReadyId=
                m_stack.last().instanceId;

        break;
    }
    case AppInfo::SingleTask:
    {
        int index=findTask(appId);

        if(index>=0)
        {
            while(m_stack.size()-1>index)
            {
                AppInstance dead=
                        m_stack.takeLast();

                changeState(
                            dead,
                            AppState::Destroyed);

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

        m_pendingReadyId=
                m_stack.last().instanceId;

        break;
    }

    }
}

void PageManager::pageReady(
        quint64 instanceId)
{
    if(instanceId!=m_pendingReadyId)
        return;

    if(m_stack.isEmpty())
        return;

    m_pendingReadyId=0;

    AppInstance &instance=
            m_stack.last();

    changeState(
                instance,
                AppState::Ready);

    changeState(
                instance,
                AppState::Foreground);

    instance.firstLaunch=false;
}

void PageManager::back()
{
    if(m_stack.size()<=1)
    {
        qCInfo(logPageManager)
                << "Back ignored";

        return;
    }

    AppInstance dead=
            m_stack.takeLast();

    changeState(
                dead,
                AppState::Destroyed);

    emit instanceDestroyed(
                dead.instanceId,
                dead.info.appId);

    changeState(
                m_stack.last(),
                AppState::Foreground);

    emit currentChanged();
}
