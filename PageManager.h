#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <QObject>
#include <QVector>

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

    Q_INVOKABLE void launch(const QString &appId);

    Q_INVOKABLE void back();

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
    AppInstance createInstance(const AppInfo &info);

    void changeState(AppInstance &instance, AppState::State state);

    int findTask(const QString &appId) const;

    int findBackground(const QString &appId) const;

    AppInstance *findInstance(quint64 instanceId);

private:
    AppRegistry *m_registry = nullptr;

    QVector<AppInstance> m_stack;

    QVector<AppInstance> m_backgroundCache;

    quint64 m_nextInstanceId = 1;
};

#endif
