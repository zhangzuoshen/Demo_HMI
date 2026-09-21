#ifndef SCENECONTAINER_H
#define SCENECONTAINER_H

#include <QHash>
#include <QQuickItem>

#include "AppInstance.h"

class PageManager;
class PageView;

class SceneContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* pageManager
               READ pageManager
               WRITE setPageManager
               NOTIFY pageManagerChanged)

public:

    explicit SceneContainer(QQuickItem *parent=nullptr);
    ~SceneContainer() override;

    QObject *pageManager() const;
    void setPageManager(QObject *mgr);

signals:
    void pageManagerChanged();

protected:
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;

private slots:

    void onSceneCreated(AppInstance instance);
    void onSceneAttached(quint64 instanceId);
    void onSceneDetached(quint64 instanceId);
    void onSceneDestroyed(quint64 instanceId);

private:

    PageView *findView(quint64 instanceId);
    PageView *createView(const AppInstance &instance);

private:

    QObject *m_pageManager=nullptr;

    QHash<quint64,PageView*> m_cachedViews;

    PageView *m_frontView=nullptr;
};

#endif
