#include "SceneContainer.h"

#include <QDebug>

#include "PageManager.h"
#include "Log.h"

class SceneContainer::SceneIncubator
        : public QQmlIncubator
{
public:
    explicit SceneIncubator(SceneContainer *owner)
        : QQmlIncubator(Asynchronous),
          m_owner(owner)
    {
    }

protected:
    void statusChanged(Status status) override
    {
        switch(status)
        {
        case Ready:
            m_owner->finishCreate(object());
            break;
        case Error:
            m_owner->createError();
            break;
        default:
            break;
        }
    }

private:
    SceneContainer *m_owner;
};

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
    unload();
}

QObject* SceneContainer::pageManager() const
{
    return m_pageManager;
}

void SceneContainer::setPageManager(QObject *mgr)
{
    if(m_pageManager==mgr)
        return;

    if(m_pageManager)
    {
        disconnect(m_pageManager, nullptr, this, nullptr);
    }

    m_pageManager=mgr;

    if(m_pageManager) {
        connect(m_pageManager, SIGNAL(currentAppChanged()),
                this, SLOT(onCurrentAppChanged()));

        onCurrentAppChanged();
    }

    emit pageManagerChanged();
}

void SceneContainer::onCurrentAppChanged()
{
    auto mgr = qobject_cast<PageManager*>(m_pageManager);

    if(!mgr) {
        return;
    }

    load(mgr->currentApp());
}

void SceneContainer::load(const AppInstance &instance)
{
    unload();
    m_currentInstance = instance;
    m_engine = qmlEngine(this);

    if(!m_engine)
    {
        qCWarning(logSceneContainer)
                << "SceneContainer has no QQmlEngine";
        return;
    }

    qCInfo(logSceneContainer)
            << "Scene load:"
            << instance.info.appId
            << "#" << instance.instanceId;

    m_context = new QQmlContext(m_engine->rootContext(), this);
    m_appContext = new AppContext(instance, m_context);

    m_context->setContextProperty(
                "AppContext",
                m_appContext);

    QString path = instance.info.basePath;
    path.replace(":/","qrc:/");

    m_component = new QQmlComponent(m_engine,
                                    QUrl(path+"/"+instance.info.entry),
                                    this);

    if(m_component->isError())
    {
        qWarning()<<m_component->errors();
        return;
    }

    m_incubator = new SceneIncubator(this);
    m_component->create(*m_incubator, m_context);
}

void SceneContainer::finishCreate(QObject *object)
{
    m_rootItem = qobject_cast<QQuickItem*>(object);

    if(!m_rootItem)
    {
        delete object;
        return;
    }

    m_rootItem->setParent(this);
    m_rootItem->setParentItem(this);

    m_rootItem->setWidth(width());
    m_rootItem->setHeight(height());

    auto mgr = qobject_cast<PageManager*>(m_pageManager);

    if(mgr)
    {
        mgr->pageReady(m_currentInstance.instanceId);
    }
}

void SceneContainer::createError()
{
    if(!m_component)
        return;

    qWarning()<<m_component->errors();
}

void SceneContainer::unload()
{
    if(m_rootItem)
    {
        m_rootItem->deleteLater();
        m_rootItem=nullptr;
    }

    delete m_incubator;
    m_incubator=nullptr;

    if(m_component)
    {
        m_component->deleteLater();
        m_component=nullptr;
    }

    if(m_context)
    {
        m_context->deleteLater();
        m_context=nullptr;
        m_appContext=nullptr;
    }
}

void SceneContainer::geometryChanged(
        const QRectF &newGeometry,
        const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(
                newGeometry,
                oldGeometry);

    if(m_rootItem)
    {
        m_rootItem->setWidth(newGeometry.width());
        m_rootItem->setHeight(newGeometry.height());
    }
}
