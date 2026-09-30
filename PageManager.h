#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QList>

#include "AppRegistry.h"
#include "AppContext.h"
#include "AppInstance.h"
#include "AppState.h"

class PageManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentAppId READ currentAppId NOTIFY currentChanged)

    Q_PROPERTY(
        quint64 currentInstanceId READ currentInstanceId NOTIFY currentChanged)

    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY currentChanged)

public:
    explicit PageManager(AppRegistry *registry, QObject *parent = nullptr);
    ~PageManager() override;

    Q_INVOKABLE void launch(const QString &appId,
                            const QVariantMap &args = QVariantMap());

    Q_INVOKABLE void back();

    // 供 PopupManager 调用：模态弹窗开合时把栈顶页面置 Covered / 恢复 Foreground。
    // AppState::Covered 本就是"被覆盖"语义，页面可借此暂停动画、视频解码。
    void setTopPageCovered(bool covered);

    QString currentAppId() const;

    quint64 currentInstanceId() const;

    bool canGoBack() const;

public slots:
    void pageReady(quint64 instanceId);

signals:
    void sceneCreated(const AppInstance &instance);

    void sceneAttached(quint64 instanceId);

    void sceneDetached(quint64 instanceId);

    void sceneDestroyed(quint64 instanceId);

    void currentChanged();

private:
    AppInstance createInstance(const AppInfo &info, const QVariantMap &args);

    void changeState(AppInstance &instance, AppState::State state);

    int findTask(const QString &appId) const;

    int findBackground(const QString &appId) const;

    AppInstance *findInstance(quint64 instanceId);

    // 释放实例的 AppContext：已被 QML 根对象接管的交给对象树，
    // 未接管（QML 创建失败）的自行兜底释放
    void releaseContext(AppInstance &instance);

private:
    AppRegistry *m_registry = nullptr;

    QList<AppInstance> m_stack;

    QList<AppInstance> m_backgroundCache;

    quint64 m_nextInstanceId = 1;
};

#endif
