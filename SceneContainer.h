#ifndef SCENECONTAINER_H
#define SCENECONTAINER_H

#include <QHash>
#include <QQuickItem>

#include "AppInstance.h"

class PageView;

class SceneContainer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* pageManager READ pageManager WRITE setPageManager)

public:
    explicit SceneContainer(QQuickItem *parent = nullptr);
    ~SceneContainer() override;

    void setPageManager(QObject *manager);
    QObject *pageManager() const;

protected:
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;

private slots:
    void onSceneCreated(const AppInstance &instance);
    void onSceneAttached(quint64 instanceId);
    void onSceneDetached(quint64 instanceId);
    void onSceneDestroyed(quint64 instanceId);

private:
    PageView *createView(const AppInstance &instance);
    PageView *findView(quint64 instanceId) const;

private:
    QObject *m_pageManager = nullptr;

    QHash<quint64, PageView*> m_cachedViews;

    PageView *m_frontView = nullptr;

    // 当前真正等待显示的实例（修复异步孵化竞态）
    quint64 m_pendingFrontId = 0;
};

#endif
