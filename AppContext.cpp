#include "AppContext.h"

#include "AppInstance.h"

AppContext::AppContext(QObject *parent)
    : QObject(parent)
{
}

void AppContext::initialize(const AppInstance &instance)
{
    m_appId = instance.info.appId;
    m_appName = instance.info.name;
    m_entry = instance.info.entry;
    m_icon = instance.info.icon;

    m_priority = instance.info.priority;

    m_instanceId = instance.instanceId;

    m_firstLaunch = instance.firstLaunch;

    m_state = instance.state;

    switch(instance.info.launchMode)
    {
    case AppInfo::Standard:
        m_launchMode="standard";
        break;

    case AppInfo::SingleTop:
        m_launchMode="singleTop";
        break;

    case AppInfo::SingleTask:
        m_launchMode="singleTask";
        break;
    }
}

QString AppContext::appId() const{return m_appId;}
QString AppContext::appName() const{return m_appName;}
QString AppContext::entry() const{return m_entry;}
QString AppContext::icon() const{return m_icon;}
int AppContext::priority() const{return m_priority;}
QString AppContext::launchMode() const{return m_launchMode;}
quint64 AppContext::instanceId() const{return m_instanceId;}
bool AppContext::firstLaunch() const{return m_firstLaunch;}
AppState::State AppContext::state() const{return m_state;}

void AppContext::setState(AppState::State state)
{
    if(m_state==state)
        return;

    m_state=state;

    emit stateChanged();
}
