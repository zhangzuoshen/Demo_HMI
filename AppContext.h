#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>
#include "AppState.h"
#include "WindowState.h"

class AppContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString appId
               READ appId
               NOTIFY appIdChanged)

    Q_PROPERTY(QString appName
               READ appName
               NOTIFY appNameChanged)

    Q_PROPERTY(quint64 instanceId
               READ instanceId
               NOTIFY instanceIdChanged)

    Q_PROPERTY(AppState::State state
               READ state
               NOTIFY stateChanged)

    Q_PROPERTY(WindowState::State windowState
               READ windowState
               NOTIFY windowStateChanged)

public:
    explicit AppContext(QObject *parent = nullptr);

    QString appId() const;
    QString appName() const;
    quint64 instanceId() const;

    AppState::State state() const;
    WindowState::State windowState() const;

    void setAppId(const QString &id);
    void setAppName(const QString &name);
    void setInstanceId(quint64 id);

    void setState(AppState::State state);
    void setWindowState(WindowState::State state);

signals:
    void appIdChanged();
    void appNameChanged();
    void instanceIdChanged();
    void stateChanged();
    void windowStateChanged();

private:
    QString m_appId;
    QString m_appName;
    quint64 m_instanceId = 0;

    AppState::State m_state = AppState::Created;
    WindowState::State m_windowState = WindowState::Detached;
};

#endif // APPCONTEXT_H
