#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>

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
                         QObject *parent = nullptr);

    QString currentAppId() const;
    quint64 currentInstanceId() const;

    AppInstance currentApp() const;
    const QVector<AppInstance> &stack() const;

    Q_INVOKABLE void launchApp(const QString &appId,
                               LaunchReason reason = User);

    Q_INVOKABLE void back();

    // 首次创建完成（只触发一次）
    void pageReady(quint64 instanceId);

    // SceneContainer Attach 完成后调用
    void pageAttached(quint64 instanceId);

signals:

    // QML Loader 切换
    void currentChanged();

    // App 生命周期（QML BasePage 使用）
    void appEntered(quint64 instanceId);
    void appPaused(quint64 instanceId);
    void appResumed(quint64 instanceId);
    void appExited(quint64 instanceId);
    void appReady(quint64 instanceId);

    // Instance 生命周期
    void instanceCreated(quint64 instanceId,
                         QString appId);

    void instanceDestroyed(quint64 instanceId,
                           QString appId);

    // Scene 生命周期（SceneContainer 使用）
    void sceneCreated(AppInstance instance);
    void sceneAttached(quint64 instanceId);
    void sceneDetached(quint64 instanceId);
    void sceneDestroyed(quint64 instanceId);

private:

    AppInstance createInstance(const AppInfo &info);

    // 只修改状态，不发送生命周期
    void setState(AppInstance &instance,
                  AppState::State state);

    // 根据状态发送生命周期事件
    void dispatchLifecycle(AppInstance &instance,
                           AppState::State state);

    AppInstance *findInstance(quint64 instanceId);
    const AppInstance *findInstance(quint64 instanceId) const;

    int findTask(const QString &appId) const;

private:

    AppRegistry *m_registry = nullptr;

    // 前台任务栈
    QVector<AppInstance> m_stack;

    // keepAlive 后台缓存
    QHash<QString, AppInstance> m_backgroundApps;

    quint64 m_nextInstanceId = 1;
};

#endif
