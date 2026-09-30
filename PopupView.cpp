#include "PopupView.h"

#include <QQmlError>

#include "Log.h"
#include "PopupContext.h"
#include "PopupIncubator.h"

PopupView::PopupView(QQmlEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

PopupView::~PopupView()
{
    destroy();
}

bool PopupView::create(const PopupInstance &instance,
                       QQmlIncubator::IncubationMode mode)
{
    destroy();

    m_destroying = false;
    m_ready = false;
    m_attached = false;

    m_instance = instance;

    if (!m_engine)
        return false;

    m_context = new QQmlContext(m_engine->rootContext(), this);

    m_context->setContextProperty("PopupContext", instance.context.data());

    QString path = instance.info.basePath;
    path.replace(":/", "qrc:/");

    m_component = new QQmlComponent(
        m_engine, QUrl(path + "/" + instance.info.entry), this);

    m_incubator = new PopupIncubator(this, mode);

    m_component->create(*m_incubator, m_context);

    if (m_component->isError())
    {
        qCCritical(logPopup) << "Create failed:" << instance.key
                             << m_component->errors();
    }

    return true;
}

void PopupView::destroy()
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
            qCInfo(logPopup) << "Incubation cancelled:" << m_instance.key
                             << "#" << m_instance.popupId;

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

    m_destroying = false;
}

bool PopupView::isReady() const
{
    return m_ready;
}

void PopupView::attach(QQuickItem *parent)
{
    if (!m_rootItem)
        return;

    m_rootItem->setParentItem(parent);
    m_attached = true;
}

void PopupView::detach()
{
    if (!m_rootItem)
        return;

    m_rootItem->setParentItem(nullptr);
    m_attached = false;
}

void PopupView::resize(const QSizeF &size)
{
    if (!m_rootItem)
        return;

    m_rootItem->setSize(size);
}

void PopupView::setZ(qreal z)
{
    if (!m_rootItem)
        return;

    m_rootItem->setZ(z);
}

const PopupInstance &PopupView::instance() const
{
    return m_instance;
}

void PopupView::onIncubationReady()
{
    if (m_destroying)
        return;

    if (!m_incubator)
        return;

    if (m_ready)
        return;

    QObject *obj = m_incubator->object();
    QQuickItem *rootItem = qobject_cast<QQuickItem *>(obj);

    if (!rootItem)
    {
        // 所有权已从 incubator 移交给 PopupView，必须自行释放；
        // 同时上报失败，让 PopupContainer 把该 view 移出缓存
        if (obj)
            obj->deleteLater();

        emit incubationFailed(m_instance.popupId);
        return;
    }

    m_rootItem = rootItem;

    //==============================
    // 所有权移交：PopupContext 挂到 QML 根对象下
    //==============================
    // 根对象析构时会带走其子对象，两者生命周期从此同步，
    // QML 绑定不会再看到 PopupContext 变成 null。
    if (m_instance.context)
        m_instance.context->setParent(m_rootItem);

    m_ready = true;

    emit incubationReady(m_instance.popupId);
}

void PopupView::onIncubationError(const QList<QQmlError> &errors)
{
    if (m_destroying)
        return;

    qCCritical(logPopup) << "Incubation failed:" << m_instance.key << errors;

    emit incubationFailed(m_instance.popupId);
}
