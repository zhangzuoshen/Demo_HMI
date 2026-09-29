#ifndef OVERLAYCONTEXT_H
#define OVERLAYCONTEXT_H

#include <QObject>

#include "OverlayState.h"

/**
 * @brief Dialog 与 Toast 的 Context 基类
 *
 * 只管一件事：状态机 + "退场动画播完才销毁"的通知入口。
 * 子类（PopupContext / ToastContext）各自加自己的属性。
 *
 * 基类存在的意义：notifyClosed() 这条关键约定只有一份实现，
 * 不会在两个系统里各写一遍然后悄悄漂移。
 */
class OverlayContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(OverlayState::State state READ state NOTIFY stateChanged)

public:
    explicit OverlayContext(QObject *parent = nullptr);

    OverlayState::State state() const;

    // QML 侧调用：退场动画播完后通知，此刻才允许真正销毁。
    // 子类 Manager 监听 closed() 信号执行销毁。
    Q_INVOKABLE void notifyClosed();

    void setState(OverlayState::State state);

signals:
    void closed();

    void stateChanged();

private:
    OverlayState::State m_state = OverlayState::None;
};

#endif // OVERLAYCONTEXT_H
