#include "PageView.h"

#include <QQmlError>

#include "AppContext.h"
#include "Log.h"
#include "PageIncubator.h"

PageView::PageView(QQmlEngine *engine,
                   QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

PageView::~PageView()
{
    destroy();
}

bool PageView::create(const AppInstance &instance)
{
    destroy();

    m_destroying = false;
    m_ready = false;
    m_attached = false;

    m_instance = instance;

    if (!m_engine)
        return false;

    m_context = new QQmlContext(
                m_engine->rootContext(),
                this);

    m_context->setContextProperty(
                "AppContext",
                instance.context);

    QString path = instance.info.basePath;
    path.replace(":/", "qrc:/");

    m_component = new QQmlComponent(
                m_engine,
                QUrl(path + "/" + instance.info.entry),
                this);

    m_incubator = new PageIncubator(this);

    m_component->create(
                *m_incubator,
                m_context);

    setWindowState(WindowState::Creating);

    return true;
}

void PageView::destroy()
{
    if (m_destroying)
        return;

    m_destroying = true;

    m_attached = false;
    m_ready = false;

    //==============================
    // 取消异步孵化
    //==============================
    if (m_incubator)
    {
        if (m_incubator->status() == QQmlIncubator::Loading)
        {
            qCInfo(logScene)
                    << "Incubation cancelled:"
                    << m_instance.info.appId
                    << "#"
                    << m_instance.instanceId;

            m_incubator->clear();
        }

        delete m_incubator;
        m_incubator = nullptr;
    }

    //==============================
    // RootItem
    //==============================
    if (m_rootItem)
    {
        m_rootItem->setParentItem(nullptr);
        m_rootItem->deleteLater();
        m_rootItem = nullptr;
    }

    //==============================
    // Component
    //==============================
    if (m_component)
    {
        m_component->deleteLater();
        m_component = nullptr;
    }

    //==============================
    // Context
    //==============================
    if (m_context)
    {
        m_context->deleteLater();
        m_context = nullptr;
    }

    setWindowState(WindowState::Detached);

    m_destroying = false;
}

bool PageView::isReady() const
{
    return m_ready;
}

void PageView::attach(QQuickItem *parent)
{
    if (!m_rootItem)
        return;

    m_rootItem->setParent(parent);
    m_rootItem->setParentItem(parent);

    m_attached = true;

    setWindowState(WindowState::Attached);
}

void PageView::detach()
{
    if (!m_rootItem)
        return;

    m_rootItem->setParentItem(nullptr);

    m_attached = false;

    setWindowState(WindowState::Detached);
}

bool PageView::isAttached() const
{
    return m_attached;
}

void PageView::resize(const QSizeF &size)
{
    if (!m_rootItem)
        return;

    m_rootItem->setSize(size);
}

const AppInstance &PageView::instance() const
{
    return m_instance;
}

WindowState::State PageView::windowState() const
{
    return m_windowState;
}

void PageView::onIncubationReady()
{
    if (m_destroying)
        return;

    if (!m_incubator)
        return;

    QObject *obj = m_incubator->object();

    m_rootItem = qobject_cast<QQuickItem *>(obj);

    if (!m_rootItem)
        return;

    m_ready = true;

    setWindowState(WindowState::Ready);

    emit incubationReady(m_instance.instanceId);
}

void PageView::onIncubationError(const QList<QQmlError> &errors)
{
    if (m_destroying)
        return;

    qCCritical(logScene)
            << "Incubation failed:"
            << m_instance.info.appId
            << errors;

    emit incubationFailed(m_instance.instanceId);
}

void PageView::setWindowState(WindowState::State state)
{
    if (m_windowState == state)
        return;

    m_windowState = state;
}
