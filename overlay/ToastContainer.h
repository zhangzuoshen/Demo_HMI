#ifndef TOASTCONTAINER_H
#define TOASTCONTAINER_H

#include <QList>
#include <QQuickItem>

class ToastManager;

/**
 * @brief Toast 的容器与布局器
 *
 * 布局在 C++ 里手算，不用 QML 的 Column：Column 是定位器，会接管所有
 * childItems 的 y，而 Toast 的滑入滑出动画也要控制 y，两者会打架。
 *
 * 混合 position（top / bottom / center）时按 position 分组，组内堆叠；
 * 组内按 priority 降序，高优先级排在最靠近锚边（最显眼）的位置。
 */
class ToastContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject *toastManager READ toastManager WRITE setToastManager)

    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

    Q_PROPERTY(int margin READ margin WRITE setMargin)

public:
    explicit ToastContainer(QQuickItem *parent = nullptr);
    ~ToastContainer() override;

    void setToastManager(QObject *manager);
    QObject *toastManager() const;

    int spacing() const;
    void setSpacing(int spacing);

    int margin() const;
    void setMargin(int margin);

    // position / priority 来自 ToastInfo，供分组与排序用
    void addItem(QQuickItem *item, int position, int priority);
    void removeItem(QQuickItem *item);

    void relayout();

protected:
    void geometryChange(const QRectF &newGeometry,
                        const QRectF &oldGeometry) override;

private:
    struct ItemInfo
    {
        QQuickItem *item = nullptr;

        // ToastInfo::Position
        int position = 1;

        int priority = 50;
    };

private:
    QObject *m_toastManager = nullptr;

    QList<ItemInfo> m_items;

    int m_spacing = 8;

    int m_margin = 24;
};

#endif // TOASTCONTAINER_H
