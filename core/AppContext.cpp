#include "AppContext.h"

AppContext::AppContext(QObject *parent)
    : QObject(parent)
{
}

QString AppContext::appId() const
{
    return m_appId;
}

QString AppContext::appName() const
{
    return m_appName;
}

quint64 AppContext::instanceId() const
{
    return m_instanceId;
}

AppState::State AppContext::state() const
{
    return m_state;
}

WindowState::State AppContext::windowState() const
{
    return m_windowState;
}

void AppContext::setAppId(const QString &id)
{
    if (m_appId == id)
        return;

    m_appId = id;
    emit appIdChanged();
}

void AppContext::setAppName(const QString &name)
{
    if (m_appName == name)
        return;

    m_appName = name;
    emit appNameChanged();
}

void AppContext::setInstanceId(quint64 id)
{
    if (m_instanceId == id)
        return;

    m_instanceId = id;
    emit instanceIdChanged();
}

void AppContext::setState(AppState::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged();
}

void AppContext::setWindowState(WindowState::State state)
{
    if (m_windowState == state)
        return;

    m_windowState = state;
    emit windowStateChanged();
}

QVariantMap AppContext::takeLaunchArgs() const
{
    return m_launchArgs;
}

void AppContext::setLaunchArgs(const QVariantMap &args)
{
    m_launchArgs = args;
}

void AppContext::clearLaunchArgs()
{
    m_launchArgs.clear();
}
