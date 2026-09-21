#include "SceneContainer.h"

#include <QQmlEngine>
#include <QQmlError>

#include "AppContext.h"
#include "PageManager.h"
#include "Log.h"

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
    unload();
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
        disconnect(m_pageManager,
                   nullptr,
                   this,
                   nullptr);
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

void SceneContainer::onCurrentChanged()
{
    auto mgr =
            qobject_cast<PageManager *>(m_pageManager);

    if (!mgr)
        return;

    AppInstance instance =
            mgr->currentApp();

    if (instance.instanceId == 0)
        return;

    load(instance);
}

void SceneContainer::load(const AppInstance &instance)
{
    unload();

    m_currentInstance = instance;

    m_engine = qmlEngine(this);

    if (!m_engine)
        return;

    qCInfo(logScene)
            << "Load:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    // 创建独立 Context
    m_context =
            new QQmlContext(
                m_engine->rootContext(),
                this);

    m_context->setContextProperty(
                "AppContext",
                instance.context);

    // 创建组件
    QString path =
            instance.info.basePath;

    path.replace(":/", "qrc:/");

    m_component =
            new QQmlComponent(
                m_engine,
                QUrl(path + "/" + instance.info.entry),
                this);

    if (m_component->isError())
    {
        qWarning()
                << m_component->errors();

        return;
    }

    // 创建页面
    QObject *obj =
            m_component->create(m_context);

    m_rootItem =
            qobject_cast<QQuickItem *>(obj);

    if (!m_rootItem)
    {
        delete obj;
        return;
    }

    m_rootItem->setParent(this);

    m_rootItem->setParentItem(this);

    m_rootItem->setWidth(width());

    m_rootItem->setHeight(height());

    // 通知 Ready
    auto mgr =
            qobject_cast<PageManager *>(m_pageManager);

    if (mgr)
    {
        qCInfo(logScene)
                << "Notify pageReady:"
                << instance.instanceId;

        mgr->pageReady(instance.instanceId);
    }
}

void SceneContainer::unload()
{
    if (m_rootItem)
    {
        m_rootItem->deleteLater();
        m_rootItem = nullptr;
    }

    if (m_component)
    {
        m_component->deleteLater();
        m_component = nullptr;
    }

    if (m_context)
    {
        m_context->deleteLater();
        m_context = nullptr;
    }
}

void SceneContainer::geometryChanged(
        const QRectF &newGeometry,
        const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(
                newGeometry,
                oldGeometry);

    if (m_rootItem)
    {
        m_rootItem->setWidth(
                    newGeometry.width());

        m_rootItem->setHeight(
                    newGeometry.height());
    }
}

