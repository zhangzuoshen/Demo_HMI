#include "SceneContainer.h"

#include <QQmlEngine>

#include "PageManager.h"
#include "PageView.h"
#include "Log.h"

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
    qDeleteAll(m_cachedViews);
}

QObject *SceneContainer::pageManager() const
{
    return m_pageManager;
}

void SceneContainer::setPageManager(QObject *mgr)
{
    if (m_pageManager == mgr)
        return;

    if (m_pageManager)
        disconnect(m_pageManager,nullptr,this,nullptr);

    m_pageManager = mgr;

    auto pm = qobject_cast<PageManager*>(mgr);

    if (pm)
    {
        connect(pm,&PageManager::sceneCreated,
                this,&SceneContainer::onSceneCreated);

        connect(pm,&PageManager::sceneAttached,
                this,&SceneContainer::onSceneAttached);

        connect(pm,&PageManager::sceneDestroyed,
                this,&SceneContainer::onSceneDestroyed);
    }

    emit pageManagerChanged();
}

PageView *SceneContainer::findView(quint64 instanceId)
{
    return m_cachedViews.value(instanceId,nullptr);
}

PageView *SceneContainer::createView(const AppInstance &instance)
{
    PageView *view =
            new PageView(qmlEngine(this),this);

    if (!view->create(instance))
    {
        delete view;
        return nullptr;
    }

    view->resize(QSizeF(width(),height()));

    m_cachedViews.insert(instance.instanceId,view);

    return view;
}

void SceneContainer::onSceneCreated(AppInstance instance)
{
    qCInfo(logScene)
            << "Create:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    PageView *view = createView(instance);

    if (!view)
        return;

    if (m_frontView)
    {
        const AppInstance &old = m_frontView->instance();

        if (old.info.keepAlive)
        {
            qCInfo(logScene)
                    << "Detach:"
                    << old.info.appId;

            m_frontView->detach();
        }
        else
        {
            qCInfo(logScene)
                    << "Destroy:"
                    << old.info.appId;

            m_cachedViews.remove(old.instanceId);

            delete m_frontView;
        }
    }

    m_frontView = view;

    m_frontView->attach(this);

    m_frontView->resize(QSizeF(width(),height()));

    auto pm=qobject_cast<PageManager*>(m_pageManager);

    if (pm)
    {
        qCInfo(logScene)
                << "Notify pageReady:"
                << instance.instanceId;

        pm->pageReady(instance.instanceId);
    }
}

void SceneContainer::onSceneAttached(quint64 instanceId)
{
    PageView *view = findView(instanceId);

    if (!view)
        return;

    qCInfo(logScene)
            << "Attach:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    if (m_frontView && m_frontView != view)
    {
        const AppInstance &old = m_frontView->instance();

        if (old.info.keepAlive)
            m_frontView->detach();
        else
        {
            m_cachedViews.remove(old.instanceId);
            delete m_frontView;
        }
    }

    m_frontView = view;

    if (!m_frontView->isAttached())
        m_frontView->attach(this);

    m_frontView->resize(QSizeF(width(),height()));
}

void SceneContainer::onSceneDestroyed(quint64 instanceId)
{
    PageView *view = findView(instanceId);

    if (!view)
        return;

    qCInfo(logScene)
            << "Destroy:"
            << view->instance().info.appId;

    m_cachedViews.remove(instanceId);

    if (m_frontView == view)
        m_frontView = nullptr;

    delete view;
}

void SceneContainer::geometryChanged(
        const QRectF &newGeometry,
        const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry,
                                oldGeometry);

    for (auto view : m_cachedViews)
    {
        if (view)
            view->resize(newGeometry.size());
    }
}
