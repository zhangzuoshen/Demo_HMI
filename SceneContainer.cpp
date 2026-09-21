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
    m_cachedViews.clear();

    m_frontView = nullptr;
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
    {
        disconnect(m_pageManager, nullptr, this, nullptr);
    }

    m_pageManager = mgr;

    if (m_pageManager)
    {
        connect(m_pageManager,
                SIGNAL(currentChanged()),
                this,
                SLOT(onCurrentChanged()));

        onCurrentChanged();
    }

    emit pageManagerChanged();
}

PageView *SceneContainer::findView(quint64 instanceId)
{
    return m_cachedViews.value(instanceId, nullptr);
}

PageView *SceneContainer::createView(const AppInstance &instance)
{
    PageView *view = new PageView(qmlEngine(this), this);

    if (!view->create(instance))
    {
        delete view;
        return nullptr;
    }

    view->resize(QSizeF(width(), height()));

    m_cachedViews.insert(instance.instanceId, view);

    return view;
}

void SceneContainer::onCurrentChanged()
{
    auto mgr = qobject_cast<PageManager *>(m_pageManager);

    if (!mgr)
        return;

    AppInstance instance = mgr->currentApp();

    if (instance.instanceId == 0)
        return;

    PageView *next = findView(instance.instanceId);

    bool firstCreate = false;

    if (!next)
    {
        next = createView(instance);

        if (!next)
            return;

        firstCreate = true;

        qCInfo(logScene)
                << "Create:"
                << instance.info.appId
                << "#"
                << instance.instanceId;
    }
    else
    {
        qCInfo(logScene)
                << "Attach:"
                << instance.info.appId
                << "#"
                << instance.instanceId;
    }

    // ---------- 切换前台页面 ----------
    if (m_frontView && m_frontView != next)
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

    // ---------- 切换到新的前台 ----------
    m_frontView = next;

    if (!m_frontView->isAttached())
    {
        m_frontView->attach(this);
    }

    m_frontView->resize(QSizeF(width(), height()));

    // ---------- 首次创建才通知 Ready ----------
    if (firstCreate)
    {
        qCInfo(logScene)
                << "Notify pageReady:"
                << instance.instanceId;

        mgr->pageReady(instance.instanceId);
    }
}

void SceneContainer::geometryChanged(const QRectF &newGeometry,
                                     const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    const QSizeF size = newGeometry.size();

    for (auto view : m_cachedViews)
    {
        if (view)
            view->resize(size);
    }
}
