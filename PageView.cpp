#include "PageView.h"

#include <QQmlEngine>

#include "AppContext.h"
#include "Log.h"

PageView::PageView(QQmlEngine *engine,
                   QObject *parent)
    : QObject(parent),
      m_engine(engine)
{
}

PageView::~PageView()
{
    destroy();
}

WindowState::State PageView::windowState() const
{
    return m_windowState;
}

void PageView::setWindowState(WindowState::State s)
{
    if(m_windowState==s)
        return;

    m_windowState=s;

    qCInfo(logScene)
            << "Window:"
            << instance().info.appId
            << "#"
            << instance().instanceId
            << WindowState::toString(s);

    emit windowStateChanged();
}

bool PageView::create(const AppInstance &instance)
{
    destroy();

    m_instance = instance;

    if (!m_engine)
        return false;

    // 每个 App 创建独立 QQmlContext
    m_context = new QQmlContext(m_engine->rootContext(), this);
    m_context->setContextProperty("AppContext", instance.context);

    QString path = instance.info.basePath;
    path.replace(":/", "qrc:/");

    QUrl url(path + "/" + instance.info.entry);

    m_component = new QQmlComponent(m_engine, url, this);

    if (m_component->isError())
    {
        qCCritical(logScene)
                << "Load QML failed:"
                << url;

        for (const QQmlError &e : m_component->errors())
            qCCritical(logScene) << e.toString();

        destroy();
        return false;
    }

    QObject *obj = m_component->create(m_context);

    if (!obj)
    {
        qCCritical(logScene)
                << "Create QML failed:"
                << url;

        for (const QQmlError &e : m_component->errors())
            qCCritical(logScene) << e.toString();

        destroy();
        return false;
    }

    m_rootItem = qobject_cast<QQuickItem *>(obj);

    if (!m_rootItem)
    {
        qCCritical(logScene)
                << "Root object is not QQuickItem:"
                << url;

        delete obj;
        destroy();
        return false;
    }

    // QObject 父对象只设置一次
    m_rootItem->setParent(this);

    // 初始状态
    m_rootItem->setVisible(false);
    m_attached = false;

    setWindowState(WindowState::Detached);

    qCInfo(logScene)
            << "Create:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    return true;
}

void PageView::destroy()
{
    m_attached = false;

    setWindowState(WindowState::Detached);

    if (m_rootItem)
    {
        m_rootItem->setParentItem(nullptr);
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

void PageView::attach(QQuickItem *parent)
{
    if (!m_rootItem || !parent)
        return;

    if (m_rootItem->parentItem() == parent && m_attached)
        return;

    m_rootItem->setParentItem(parent);
    m_rootItem->setVisible(true);

    m_attached = true;

    setWindowState(WindowState::Attached);

    if (m_instance.context)
        m_instance.context->setWindowState(WindowState::Attached);
}

void PageView::detach()
{
    if (!m_rootItem || !m_attached)
        return;

    m_rootItem->setVisible(false);
    m_rootItem->setParentItem(nullptr);

    m_attached = false;

    setWindowState(WindowState::Detached);

    if (m_instance.context)
        m_instance.context->setWindowState(WindowState::Detached);
}

void PageView::resize(const QSizeF &size)
{
    if(!m_rootItem)
        return;

    m_rootItem->setWidth(size.width());
    m_rootItem->setHeight(size.height());
}

bool PageView::isAttached() const
{
    return m_attached;
}

QQuickItem *PageView::rootItem() const
{
    return m_rootItem;
}

const AppInstance &PageView::instance() const
{
    return m_instance;
}
