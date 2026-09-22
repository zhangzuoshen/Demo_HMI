#include "SceneContainer.h"

#include <QQmlEngine>
#include <QQuickItem>

#include "Log.h"
#include "PageManager.h"
#include "PageView.h"

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
}

void SceneContainer::setPageManager(QObject *manager)
{
    if (m_pageManager == manager)
        return;

    if (m_pageManager)
        disconnect(m_pageManager, nullptr, this, nullptr);

    m_pageManager = manager;

    auto pm = qobject_cast<PageManager *>(manager);

    if (!pm)
        return;

    connect(pm, &PageManager::sceneCreated,
            this, &SceneContainer::onSceneCreated);

    connect(pm, &PageManager::sceneAttached,
            this, &SceneContainer::onSceneAttached);

    connect(pm, &PageManager::sceneDetached,
            this, &SceneContainer::onSceneDetached);

    connect(pm, &PageManager::sceneDestroyed,
            this, &SceneContainer::onSceneDestroyed);
}

QObject *SceneContainer::pageManager() const
{
    return m_pageManager;
}

PageView *SceneContainer::findView(quint64 instanceId) const
{
    return m_cachedViews.value(instanceId, nullptr);
}

PageView *SceneContainer::createView(const AppInstance &instance)
{
    if (auto cached = findView(instance.instanceId))
        return cached;

    PageView *view =
            new PageView(qmlEngine(this), this);

    m_cachedViews.insert(instance.instanceId, view);

    connect(view,
            &PageView::incubationReady,
            this,
            [this](quint64 id)
    {
        PageView *readyView = findView(id);

        if (!readyView)
            return;

        qCInfo(logScene)
                << "Incubation Ready:"
                << readyView->instance().info.appId
                << "#"
                << id;

        // 同步窗口尺寸
        readyView->resize(QSizeF(width(), height()));

        //========================================================
        // 不是当前期望显示的前台窗口：保持后台 Ready，不抢占焦点
        //========================================================
        if (id != m_pendingFrontId)
        {
            qCInfo(logScene)
                    << "Background incubation finished:"
                    << readyView->instance().info.appId
                    << "#"
                    << id;

            return;
        }

        //========================================================
        // 当前窗口已经是前台（例如 onSceneAttached 已经等待孵化）
        //========================================================
        if (m_frontView == readyView && readyView->isAttached())
        {
            qCDebug(logScene)
                    << "Already attached:"
                    << id;
            return;
        }

        //========================================================
        // 切换前台窗口
        //========================================================
        if (m_frontView && m_frontView != readyView)
            m_frontView->detach();

        m_frontView = readyView;

        m_frontView->attach(this);

        //========================================================
        // 只有首次完成孵化时才通知 PageManager
        //========================================================
        auto pm = qobject_cast<PageManager *>(m_pageManager);

        if (pm)
            pm->pageReady(id);
    });

    connect(view,
            &PageView::incubationFailed,
            this,
            [this](quint64 id)
    {
        qCWarning(logScene)
                << "Incubation Failed:"
                << id;

        PageView *failed = findView(id);

        if (failed)
        {
            m_cachedViews.remove(id);
            failed->deleteLater();
        }
    });

    if (!view->create(instance))
    {
        m_cachedViews.remove(instance.instanceId);
        view->deleteLater();
        return nullptr;
    }

    qCInfo(logScene)
            << "Create:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    return view;
}

void SceneContainer::onSceneCreated(const AppInstance &instance)
{
    // 当前真正等待显示的页面
    m_pendingFrontId = instance.instanceId;

    PageView *view = findView(instance.instanceId);

    // 首次创建（异步孵化）
    if (!view)
    {
        createView(instance);
        return;
    }

    // KeepAlive 页面恢复
    qCInfo(logScene)
            << "Attach:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    if (m_frontView &&
        m_frontView != view)
    {
        m_frontView->detach();
    }

    m_frontView = view;

    m_frontView->resize(QSizeF(width(), height()));
    m_frontView->attach(this);
}

void SceneContainer::onSceneAttached(quint64 instanceId)
{
    // 更新当前期望显示的前台实例
    m_pendingFrontId = instanceId;

    PageView *view = findView(instanceId);

    if (!view)
    {
        qCWarning(logScene)
                << "Attach failed: view not found:"
                << instanceId;
        return;
    }

    qCInfo(logScene)
            << "Attach:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    // 切换前台窗口
    if (m_frontView && m_frontView != view)
        m_frontView->detach();

    m_frontView = view;

    // ★ 页面还没孵化完成
    if (!view->isReady())
    {
        qCInfo(logScene)
                << "Wait incubation:"
                << instanceId;
        return;
    }


    // 同步窗口尺寸
    m_frontView->resize(QSizeF(width(), height()));

    // 挂载到 Scene
    m_frontView->attach(this);
}

void SceneContainer::onSceneDetached(quint64 instanceId)
{
    PageView *view = findView(instanceId);

    if (!view)
        return;

    qCInfo(logScene)
            << "Detach:"
            << view->instance().info.appId
            << "#"
            << instanceId;

    view->detach();

    if (m_frontView == view)
        m_frontView = nullptr;
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

    view->deleteLater();
}

void SceneContainer::geometryChanged(const QRectF &newGeometry,
                                     const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    QSizeF size = newGeometry.size();

    for (auto view : m_cachedViews)
    {
        if (view)
            view->resize(size);
    }
}
