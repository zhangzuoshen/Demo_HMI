#ifndef SCENECONTAINER_H
#define SCENECONTAINER_H

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlComponent>

#include "AppContext.h"
#include "AppInstance.h"

class PageManager;

class SceneContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* pageManager
               READ pageManager
               WRITE setPageManager
               NOTIFY pageManagerChanged)

public:

    explicit SceneContainer(QQuickItem *parent=nullptr);
    ~SceneContainer();

    QObject* pageManager() const;
    void setPageManager(QObject *mgr);

signals:

    void pageManagerChanged();

protected:

    void geometryChanged(
            const QRectF &newGeometry,
            const QRectF &oldGeometry) override;

private slots:

    void onStateChanged(
            quint64 instanceId,
            int state);

    void onCurrentChanged();

private:

    void load(const AppInstance &instance);
    void unload();

private:

    QObject *m_pageManager=nullptr;

    QQmlEngine *m_engine=nullptr;

    QQmlContext *m_context=nullptr;

    AppContext *m_appContext=nullptr;

    QQmlComponent *m_component=nullptr;

    QQuickItem *m_rootItem=nullptr;

    AppInstance m_currentInstance;
};

#endif
