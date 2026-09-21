#include "SceneContainer.h"

#include <QQmlEngine>

#include "Log.h"
#include "PageManager.h"
#include "PageView.h"

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
    qDeleteAll(m_cachedViews);
    m_cachedViews.clear();
}

QObject *SceneContainer::pageManager() const
{
    return m_pageManager;
}

void SceneContainer::setPageManager(QObject *mgr)
{
    if(m_pageManager==mgr)
        return;

    if(m_pageManager)
        disconnect(m_pageManager,
                   nullptr,
                   this,
                   nullptr);

    m_pageManager=mgr;

    auto pm=qobject_cast<PageManager*>(mgr);

    if(pm)
    {
        connect(pm,
                &PageManager::sceneCreated,
                this,
                &SceneContainer::onSceneCreated);

        connect(pm,
                &PageManager::sceneAttached,
                this,
                &SceneContainer::onSceneAttached);

        connect(pm,
                &PageManager::sceneDetached,
                this,
                &SceneContainer::onSceneDetached);

        connect(pm,
                &PageManager::sceneDestroyed,
                this,
                &SceneContainer::onSceneDestroyed);
    }

    emit pageManagerChanged();
}

PageView *SceneContainer::findView(quint64 instanceId)
{
    return m_cachedViews.value(instanceId,nullptr);
}

PageView *SceneContainer::createView(const AppInstance &instance)
{
    if(auto cached=findView(instance.instanceId))
        return cached;

    PageView *view=
            new PageView(qmlEngine(this),this);

    if(!view->create(instance))
    {
        delete view;
        return nullptr;
    }

    view->resize(QSizeF(width(),height()));

    m_cachedViews.insert(instance.instanceId,
                         view);

    return view;
}

void SceneContainer::onSceneCreated(AppInstance instance)
{
    PageView *view=createView(instance);

    if(!view)
        return;

    qCInfo(logScene)
            << "Create:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    if(m_frontView &&
       m_frontView!=view)
    {
        qCInfo(logScene)
                << "Detach:"
                << m_frontView->instance().info.appId;

        m_frontView->detach();
    }

    m_frontView=view;

    if(!m_frontView->isAttached())
        m_frontView->attach(this);

    m_frontView->resize(QSizeF(width(),height()));

    auto pm=qobject_cast<PageManager*>(m_pageManager);

    if(pm)
    {
        qCInfo(logScene)
                << "Notify pageReady:"
                << instance.instanceId;

        pm->pageReady(instance.instanceId);
    }
}

void SceneContainer::onSceneAttached(quint64 instanceId)
{
    PageView *view=findView(instanceId);

    if(!view)
        return;

    qCInfo(logScene)
            << "Attach:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    if(m_frontView &&
       m_frontView!=view)
    {
        qCInfo(logScene)
                << "Detach:"
                << m_frontView->instance().info.appId;

        m_frontView->detach();
    }

    m_frontView=view;

    if(!m_frontView->isAttached())
        m_frontView->attach(this);

    m_frontView->resize(QSizeF(width(),height()));

    // -------- Resume 放到 Attach 后 --------
    auto pm=qobject_cast<PageManager*>(m_pageManager);

    if(pm)
        pm->pageAttached(instanceId);
}

void SceneContainer::onSceneDetached(quint64 instanceId)
{
    PageView *view=findView(instanceId);

    if(!view)
        return;

    qCInfo(logScene)
            << "Detach:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    if(view->isAttached())
        view->detach();

    // 注意：
    // 不清空 m_frontView。
}

void SceneContainer::onSceneDestroyed(quint64 instanceId)
{
    PageView *view = findView(instanceId);

    if (!view)
        return;

    qCInfo(logScene)
            << "Destroy:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    if (m_frontView == view)
        m_frontView = nullptr;

    m_cachedViews.remove(instanceId);

    // 不要同步 delete
    view->deleteLater();
}

void SceneContainer::geometryChanged(const QRectF &newGeometry,
                                     const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry,
                                oldGeometry);

    for(auto view:m_cachedViews)
    {
        if(view)
            view->resize(newGeometry.size());
    }
}
