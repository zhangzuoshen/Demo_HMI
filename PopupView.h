#ifndef POPUPVIEW_H
#define POPUPVIEW_H

#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSizeF>

#include "PopupInstance.h"

class PopupIncubator;

/**
 * @brief 单个弹窗的 QML 视图（PageView 的精简版）
 *
 * 与 PageView 的差异：
 * - 没有 AppState / WindowState，只有 m_ready / m_destroying
 * - 多一个 setZ()：多个弹窗同时可见，需要显式 z 序
 * - 同样在 QML 就绪后把 PopupContext 的所有权移交给 QML 根对象
 */
class PopupView : public QObject
{
    Q_OBJECT

public:
    explicit PopupView(QQmlEngine *engine, QObject *parent = nullptr);
    ~PopupView() override;

    // mode：小弹窗可用 Synchronous 避免首帧闪烁，默认异步
    bool create(const PopupInstance &instance,
                QQmlIncubator::IncubationMode mode = QQmlIncubator::Asynchronous);

    void destroy();

    bool isReady() const;

    void attach(QQuickItem *parent);
    void detach();

    void resize(const QSizeF &size);
    void setZ(qreal z);

    const PopupInstance &instance() const;

    void onIncubationReady();
    void onIncubationError(const QList<QQmlError> &errors);

signals:
    void incubationReady(quint64 popupId);
    void incubationFailed(quint64 popupId);

private:
    QQmlEngine *m_engine = nullptr;
    QQmlContext *m_context = nullptr;
    QQmlComponent *m_component = nullptr;
    PopupIncubator *m_incubator = nullptr;
    QQuickItem *m_rootItem = nullptr;

    PopupInstance m_instance;

    bool m_ready = false;
    bool m_attached = false;

    // 销毁保护：防止孵化完成回调访问已释放对象
    bool m_destroying = false;
};

#endif // POPUPVIEW_H
