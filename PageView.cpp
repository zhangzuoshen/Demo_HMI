#include "PageView.h"

#include <QQmlEngine>
#include <QQmlError>

#include "AppContext.h"

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

bool PageView::create(const AppInstance &instance)
{
    destroy();

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

    if (m_component->isError())
        return false;

    QObject *obj = m_component->create(m_context);

    m_rootItem = qobject_cast<QQuickItem *>(obj);

    if (!m_rootItem)
    {
        delete obj;
        return false;
    }

    return true;
}

void PageView::destroy()
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

void PageView::attach(QQuickItem *parentItem)
{
    if (!m_rootItem)
        return;

    m_rootItem->setParentItem(parentItem);
    m_rootItem->setParent(parentItem);
}

void PageView::detach()
{
    if (m_rootItem)
        m_rootItem->setParentItem(nullptr);
}

void PageView::resize(const QSizeF &size)
{
    if (!m_rootItem)
        return;

    m_rootItem->setWidth(size.width());
    m_rootItem->setHeight(size.height());
}

QQuickItem *PageView::rootItem() const
{
    return m_rootItem;
}

const AppInstance &PageView::instance() const
{
    return m_instance;
}
