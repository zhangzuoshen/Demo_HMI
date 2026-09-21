#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QVector>

#include "AppRegistry.h"
#include "AppInstance.h"
#include "AppState.h"

class PageManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentAppId
               READ currentAppId
               NOTIFY currentChanged)

    Q_PROPERTY(quint64 currentInstanceId
               READ currentInstanceId
               NOTIFY currentChanged)

public:

    enum LaunchReason
    {
        User,
        System
    };
    Q_ENUM(LaunchReason)

    explicit PageManager(AppRegistry *registry,
                         QObject *parent=nullptr);

    QString currentAppId() const;

    quint64 currentInstanceId() const;

    AppInstance currentApp() const;

    const QVector<AppInstance>& stack() const;

    Q_INVOKABLE void launchApp(
            const QString &appId,
            LaunchReason reason=User);

    Q_INVOKABLE void back();

    void pageReady(quint64 instanceId);

signals:

    // SceneContainer监听
    void currentChanged();

    // 生命周期状态变化
    void instanceCreated(
            quint64 instanceId,
            QString appId);

    void instanceDestroyed(
            quint64 instanceId,
            QString appId);

    void instanceStateChanged(
            quint64 instanceId,
            int state);

private:

    AppInstance createInstance(
            const AppInfo &info);

    void changeState(
            AppInstance &instance,
            AppState::State state);

    int findTask(
            const QString &appId) const;

private:

    AppRegistry *m_registry=nullptr;

    QVector<AppInstance> m_stack;

    quint64 m_nextInstanceId=1;

    quint64 m_pendingReadyId=0;
};

#endif
