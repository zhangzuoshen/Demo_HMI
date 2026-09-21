#include "PageManager.h"
#include "Log.h"

PageManager::PageManager(AppRegistry *registry,
                         QObject *parent)
    : QObject(parent),
      m_registry(registry)
{
    qCInfo(logPageManager)
            << "PageManager created";
}

QString PageManager::currentAppId() const
{
    if(m_stack.isEmpty())
        return "";

    return m_stack.last().info.appId;
}

const QVector<AppInstance>& PageManager::stack() const
{
    return m_stack;
}

AppInstance PageManager::currentApp() const
{
    if(m_stack.isEmpty())
        return AppInstance();

    return m_stack.last();
}

AppInstance PageManager::createInstance(
        const AppInfo &info)
{
    AppInstance instance;

    instance.instanceId = m_nextInstanceId++;
    instance.firstLaunch = true;
    instance.info = info;

    return instance;
}

void PageManager::launchApp(
        const QString &appId,
        LaunchReason reason)
{
    qCInfo(logPageManager)
            << "Launch request:"
            << appId;

    if(!m_registry->contains(appId))
    {
        qCWarning(logPageManager)
                << "App not found:"
                << appId;
        return;
    }

    AppInfo target = m_registry->app(appId);

    if(reason==System && !m_stack.isEmpty())
    {
        if(target.priority < m_stack.last().info.priority)
        {
            qCInfo(logPageManager)
                    << "System launch rejected by priority";

            return;
        }
    }

    switch(target.launchMode)
    {

    case AppInfo::Standard:
    {
        if(!m_stack.isEmpty())
            pause(m_stack.last());

        m_stack.append(createInstance(target));

        emit currentAppChanged();

        emit appCreated(m_stack.last().instanceId, m_stack.last().info.appId);

        m_pendingReadyId = m_stack.last().instanceId;

        return;
    }

    case AppInfo::SingleTop:
    {
        if(!m_stack.isEmpty() &&
           m_stack.last().info.appId==appId)
        {
            qCInfo(logPageManager)
                    << "SingleTop reuse";

            emit appNewIntent(m_stack.last().instanceId, m_stack.last().info.appId);

            return;
        }

        if(!m_stack.isEmpty())
            pause(m_stack.last());

        m_stack.append(createInstance(target));

        emit currentAppChanged();

        emit appCreated(m_stack.last().instanceId, m_stack.last().info.appId);

        m_pendingReadyId = m_stack.last().instanceId;

        return;
    }

    case AppInfo::SingleTask:
    {
        int index = -1;

        for(int i=0;i<m_stack.size();i++)
        {
            if(m_stack[i].info.appId==appId)
            {
                index=i;
                break;
            }
        }

        if(index!=-1)
        {
            while(m_stack.size()-1>index)
            {
                destroy(m_stack.last());
                m_stack.removeLast();
            }

            resume(m_stack.last());

            emit currentAppChanged();

            return;
        }

        if(!m_stack.isEmpty())
            pause(m_stack.last());

        m_stack.append(createInstance(target));

        emit currentAppChanged();

        emit appCreated(m_stack.last().instanceId, m_stack.last().info.appId);

        m_pendingReadyId = m_stack.last().instanceId;

        return;
    }

    }
}

void PageManager::back()
{
    if(m_stack.size()<=1)
    {
        qCInfo(logPageManager)
                << "Back ignored";

        return;
    }

    destroy(m_stack.last());

    m_stack.removeLast();

    resume(m_stack.last());

    emit currentAppChanged();
}

void PageManager::pageReady(quint64 instanceId)
{
    if(m_stack.isEmpty())
        return;

    if(instanceId!=m_pendingReadyId)
        return;

    m_pendingReadyId = 0;

    enter(m_stack.last());
}

void PageManager::enter(AppInstance &instance)
{
    qCInfo(logPageManager)
            << "[" << instance.instanceId << "] Enter:"
            << instance.info.appId;

    emit appReady(instance.instanceId, instance.info.appId);
    emit appEntered(instance.instanceId, instance.info.appId);

    instance.firstLaunch = false;
}

void PageManager::pause(AppInstance &instance)
{
    qCInfo(logPageManager)
            << "[" << instance.instanceId << "] Pause:"
            << instance.info.appId;

    emit appPaused(instance.instanceId, instance.info.appId);
}

void PageManager::resume(AppInstance &instance)
{
    qCInfo(logPageManager)
            << "[" << instance.instanceId << "] Resume:"
            << instance.info.appId;

    emit appResumed(instance.instanceId, instance.info.appId);
}

void PageManager::exit(AppInstance &instance)
{
    qCInfo(logPageManager)
            << "[" << instance.instanceId << "] Exit:"
            << instance.info.appId;

    emit appExited(instance.instanceId, instance.info.appId);
}

void PageManager::destroy(AppInstance &instance)
{
    exit(instance);

    qCInfo(logPageManager)
            << "[" << instance.instanceId << "] Destroy:"
            << instance.info.appId;

    emit appDestroyed(instance.instanceId, instance.info.appId);
}
