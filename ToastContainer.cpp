#include "ToastContainer.h"

#include <QQuickItem>

#include <algorithm>
#include <utility>

#include "Log.h"
#include "ToastManager.h"

// 与 ToastInfo::Position 对应
enum { PosTop = 0, PosBottom = 1, PosCenter = 2 };

ToastContainer::ToastContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

ToastContainer::~ToastContainer()
{
    m_items.clear();
}

void ToastContainer::setToastManager(QObject *manager)
{
    if (m_toastManager == manager)
        return;

    if (m_toastManager)
        disconnect(m_toastManager, nullptr, this, nullptr);

    m_toastManager = manager;

    auto *tm = qobject_cast<ToastManager *>(manager);

    if (!tm)
        return;

    tm->setContainer(this);
}

QObject *ToastContainer::toastManager() const
{
    return m_toastManager;
}

int ToastContainer::spacing() const
{
    return m_spacing;
}

void ToastContainer::setSpacing(int spacing)
{
    if (m_spacing == spacing)
        return;

    m_spacing = spacing;

    relayout();
}

int ToastContainer::margin() const
{
    return m_margin;
}

void ToastContainer::setMargin(int margin)
{
    if (m_margin == margin)
        return;

    m_margin = margin;

    relayout();
}

void ToastContainer::addItem(QQuickItem *item, int position, int priority)
{
    if (!item)
        return;

    for (const ItemInfo &info : std::as_const(m_items))
    {
        if (info.item == item)
            return;
    }

    item->setParentItem(this);

    ItemInfo info;

    info.item = item;
    info.position = position;
    info.priority = priority;

    m_items.append(info);

    relayout();
}

void ToastContainer::removeItem(QQuickItem *item)
{
    if (!item)
        return;

    for (int i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].item != item)
            continue;

        m_items.removeAt(i);

        if (item->parentItem() == this)
            item->setParentItem(nullptr);

        relayout();

        return;
    }
}

void ToastContainer::relayout()
{
    //==============================
    // 按 position 分组，组内堆叠
    //==============================
    for (int pos = PosTop; pos <= PosCenter; ++pos)
    {
        QList<ItemInfo> group;

        for (const ItemInfo &info : std::as_const(m_items))
        {
            if (info.position == pos)
                group.append(info);
        }

        if (group.isEmpty())
            continue;

        // 高优先级排在最靠近锚边处；同优先级保持插入顺序（stable_sort）
        std::stable_sort(group.begin(), group.end(),
                         [](const ItemInfo &a, const ItemInfo &b) {
                             return a.priority > b.priority;
                         });

        qreal total = -m_spacing;

        for (const ItemInfo &info : std::as_const(group))
            total += info.item->height() + m_spacing;

        qreal y = 0;

        switch (pos)
        {
        case PosTop:
            y = m_margin;
            break;

        case PosBottom:
            y = height() - total - m_margin;
            break;

        case PosCenter:
        default:
            y = (height() - total) / 2;
            break;
        }

        for (const ItemInfo &info : std::as_const(group))
        {
            // 水平居中；y 由容器独占，动画改的是 Translate 偏移，不冲突
            info.item->setX((width() - info.item->width()) / 2);
            info.item->setY(y);

            y += info.item->height() + m_spacing;
        }
    }
}

void ToastContainer::geometryChange(const QRectF &newGeometry,
                                    const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    relayout();
}
