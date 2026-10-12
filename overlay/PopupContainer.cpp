#include "PopupContainer.h"

#include <QQuickWindow>
#include <QQmlEngine>
#include <QQuickItem>

#include <algorithm>
#include <utility>

#include "Log.h"
#include "PopupManager.h"
#include "PopupView.h"

PopupContainer::PopupContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

PopupContainer::~PopupContainer()
{
    // view 以本对象为 QObject 父，此处只清映射，实际析构由对象树完成
    m_views.clear();
    m_order.clear();
}

void PopupContainer::setPopupManager(QObject *manager)
{
    if (m_popupManager == manager)
        return;

    if (m_popupManager)
        disconnect(m_popupManager, nullptr, this, nullptr);

    m_popupManager = manager;

    auto *pm = qobject_cast<PopupManager *>(manager);

    if (!pm)
        return;

    connect(pm, &PopupManager::popupCreated, this,
            &PopupContainer::onPopupCreated);

    connect(pm, &PopupManager::popupDestroyed, this,
            &PopupContainer::onPopupDestroyed);
}

QObject *PopupContainer::popupManager() const
{
    return m_popupManager;
}

PopupView *PopupContainer::findView(quint64 popupId) const
{
    return m_views.value(popupId, nullptr);
}

void PopupContainer::restack()
{
    //==============================
    // 必须按显式入栈顺序取出：QHash::values() 顺序不确定，
    // 同优先级时"后开的在上"这条规则会失效
    //==============================
    QList<PopupView *> sorted;

    for (quint64 id : std::as_const(m_order))
    {
        if (PopupView *view = m_views.value(id, nullptr))
            sorted.append(view);
    }

    std::stable_sort(sorted.begin(), sorted.end(),
                     [](PopupView *a, PopupView *b) {
                         return a->instance().info.priority
                                > b->instance().info.priority;
                     });

    qreal z = 0;

    for (PopupView *view : std::as_const(sorted))
        view->setZ(z += 10);
}

PopupView *PopupContainer::createView(const PopupInstance &instance)
{
    if (findView(instance.popupId))
        return findView(instance.popupId);

    PopupView *view = new PopupView(qmlEngine(this), this);

    m_views.insert(instance.popupId, view);
    m_order.append(instance.popupId);

    connect(view, &PopupView::incubationReady, this, [this](quint64 id) {
        PopupView *readyView = findView(id);

        if (!readyView)
            return;

        qCInfo(logPopup) << "Incubation ready:" << readyView->instance().key
                         << "#" << id;

        readyView->resize(QSizeF(width(), height()));
        readyView->attach(this);

        restack();

        auto *pm = qobject_cast<PopupManager *>(m_popupManager);

        if (pm)
            pm->popupReady(id);
    });

    connect(view, &PopupView::incubationFailed, this, [this](quint64 id) {
        qCWarning(logPopup) << "Incubation failed:" << id;

        if (PopupView *failed = findView(id))
        {
            m_views.remove(id);
            m_order.removeOne(id);
            failed->deleteLater();
        }

        //==============================
        // 必须让 Manager 走一遍关闭流程：popupResult 是调用方
        // callback 的唯一出口，漏发就会永久等待
        //==============================
        auto *pm = qobject_cast<PopupManager *>(m_popupManager);

        if (pm)
            pm->close(id);
    });

    if (!view->create(instance))
    {
        m_views.remove(instance.popupId);
        m_order.removeOne(instance.popupId);

        view->deleteLater();

        return nullptr;
    }

    qCInfo(logPopup) << "Create:" << instance.key << "#" << instance.popupId;

    return view;
}

void PopupContainer::onPopupCreated(const PopupInstance &instance)
{
    createView(instance);

    restack();
}

void PopupContainer::onPopupDestroyed(quint64 popupId)
{
    PopupView *view = findView(popupId);

    if (!view)
        return;

    qCInfo(logPopup) << "Destroy:" << view->instance().key << "#" << popupId;

    m_views.remove(popupId);
    m_order.removeOne(popupId);

    view->deleteLater();

    restack();
}

void PopupContainer::geometryChange(const QRectF &newGeometry,
                                    const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    QSizeF size = newGeometry.size();

    for (quint64 id : std::as_const(m_order))
    {
        if (PopupView *view = m_views.value(id, nullptr))
            view->resize(size);
    }
}
