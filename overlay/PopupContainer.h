#ifndef POPUPCONTAINER_H
#define POPUPCONTAINER_H

#include <QHash>
#include <QList>
#include <QQuickItem>

#include "PopupInstance.h"

class PopupView;

/**
 * @brief 弹窗容器（SceneContainer 的对应物）
 *
 * 与 SceneContainer 的关键差异：**没有 m_frontView**。
 * SceneContainer 是"单前台"模型（切换时 detach 旧的），
 * 弹窗是"全部可见 + z 序叠加"，所以每个实例建一个 view 后全部 attach，
 * 由 restack() 统一按 (优先级降序, 入栈序) 排 z。
 */
class PopupContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject *popupManager READ popupManager WRITE setPopupManager)

public:
    explicit PopupContainer(QQuickItem *parent = nullptr);
    ~PopupContainer() override;

    void setPopupManager(QObject *manager);
    QObject *popupManager() const;

protected:
    void geometryChange(const QRectF &newGeometry,
                        const QRectF &oldGeometry) override;

private slots:
    void onPopupCreated(const PopupInstance &instance);
    void onPopupDestroyed(quint64 popupId);

private:
    PopupView *createView(const PopupInstance &instance);
    PopupView *findView(quint64 popupId) const;

    // 按 (优先级降序, 入栈序) 刷新 z 序
    void restack();

private:
    QObject *m_popupManager = nullptr;

    QHash<quint64, PopupView *> m_views;

    // 入栈顺序。QHash 的遍历顺序不确定，z 序必须依赖这份显式顺序
    QList<quint64> m_order;
};

#endif // POPUPCONTAINER_H
