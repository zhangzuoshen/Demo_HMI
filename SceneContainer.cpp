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
        disconnect(m_pageManager,
                   nullptr,
                   this,
                   nullptr);

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

void SceneContainer::onCurrentChanged()
{
    auto mgr = qobject_cast<PageManager *>(m_pageManager);

    if (!mgr)
        return;

    AppInstance instance = mgr->currentApp();

    if (instance.instanceId == 0)
        return;

    qCInfo(logScene)
            << "Load:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    m_currentView.reset(
            new PageView(qmlEngine(this)));

    if (!m_currentView->create(instance))
        return;

    m_currentView->attach(this);

    m_currentView->resize(
            QSizeF(width(), height()));

    qCInfo(logScene)
            << "Notify pageReady:"
            << instance.instanceId;

    mgr->pageReady(instance.instanceId);
}

void SceneContainer::geometryChanged(
        const QRectF &newGeometry,
        const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(
                newGeometry,
                oldGeometry);

    if (m_currentView)
    {
        m_currentView->resize(
                newGeometry.size());
    }
}
