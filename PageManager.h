#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QVector>

#include "AppRegistry.h"
#include "AppInstance.h"

class PageManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentAppId
               READ currentAppId
               NOTIFY currentAppChanged)

public:

    enum LaunchReason
    {
        User,
        System
    };
    Q_ENUM(LaunchReason)

    explicit PageManager(AppRegistry *registry,
                         QObject *parent = nullptr);

    QString currentAppId() const;

    AppInstance currentApp() const;

    const QVector<AppInstance>& stack() const;

    Q_INVOKABLE void launchApp(
            const QString &appId,
            LaunchReason reason = User);

    Q_INVOKABLE void back();

    void pageReady(quint64 instanceId);

signals:
    void currentAppChanged();

    void appCreated(quint64 instanceId, QString appId);
    void appReady(quint64 instanceId, QString appId);
    void appEntered(quint64 instanceId, QString appId);
    void appPaused(quint64 instanceId, QString appId);
    void appResumed(quint64 instanceId, QString appId);
    void appExited(quint64 instanceId, QString appId);
    void appDestroyed(quint64 instanceId, QString appId);
    void appNewIntent(quint64 instanceId, QString appId);

private:
    AppInstance createInstance(const AppInfo &info);

    void enter(AppInstance &instance);
    void pause(AppInstance &instance);
    void resume(AppInstance &instance);
    void exit(AppInstance &instance);
    void destroy(AppInstance &instance);

private:
    AppRegistry *m_registry;
    QVector<AppInstance> m_stack;
    quint64 m_nextInstanceId = 1;
    quint64 m_pendingReadyId = 0;
};

#endif
