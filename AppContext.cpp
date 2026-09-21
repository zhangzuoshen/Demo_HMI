#include "AppContext.h"

AppContext::AppContext(const AppInstance &instance,
                       QObject *parent)
    : QObject(parent),
      m_instance(instance)
{
    switch (instance.info.launchMode)
    {
    case AppInfo::Standard:
        m_launchModeString = "standard";
        break;

    case AppInfo::SingleTop:
        m_launchModeString = "singleTop";
        break;

    case AppInfo::SingleTask:
        m_launchModeString = "singleTask";
        break;
    }
}

AppContext::State AppContext::state() const
{
    return m_state;
}

bool AppContext::isForeground() const
{
    return m_state==Foreground;
}

bool AppContext::isActive() const
{
    return m_state==Foreground;
}

void AppContext::setState(State state)
{
    if(m_state==state)
        return;

    m_state=state;

    emit stateChanged();
}

quint64 AppContext::instanceId() const
{
    return m_instance.instanceId;
}

bool AppContext::firstLaunch() const
{
    return m_instance.firstLaunch;
}

QString AppContext::appId() const
{
    return m_instance.info.appId;
}

QString AppContext::appName() const
{
    return m_instance.info.name;
}

QString AppContext::entry() const
{
    return m_instance.info.entry;
}

QString AppContext::icon() const
{
    return m_instance.info.icon;
}

int AppContext::priority() const
{
    return m_instance.info.priority;
}

QString AppContext::launchMode() const
{
    return m_launchModeString;
}
