#ifndef OVERLAYSTATE_H
#define OVERLAYSTATE_H

#include <QObject>
#include <QString>

/**
 * @brief Dialog 与 Toast 共用的生命周期状态
 *
 * 两个系统都必须遵守同一条约定：
 *   "退场动画播完才销毁"（Closing → notifyClosed() → 真正释放）。
 * 共用同一套状态机，避免这条约定在两处实现里漂移。
 */
class OverlayState
{
    Q_GADGET

public:
    enum State
    {
        None = 0,

        // 已发起创建（Dialog 为异步孵化中，QML 尚未就绪）
        Creating,

        // QML 已完成初始化，尚未挂载
        Ready,

        // 已挂载，入场动画播放中
        Opening,

        // 可见可交互
        Opened,

        // 已请求关闭，退场动画播放中
        Closing,

        // 已销毁
        Destroyed
    };
    Q_ENUM(State)

    static QString toString(State state)
    {
        switch (state)
        {
        case None: return QStringLiteral("OverlayState::None");

        case Creating: return QStringLiteral("OverlayState::Creating");

        case Ready: return QStringLiteral("OverlayState::Ready");

        case Opening: return QStringLiteral("OverlayState::Opening");

        case Opened: return QStringLiteral("OverlayState::Opened");

        case Closing: return QStringLiteral("OverlayState::Closing");

        case Destroyed: return QStringLiteral("OverlayState::Destroyed");
        }

        return QStringLiteral("OverlayState::Unknown");
    }

    static bool isAlive(State state)
    {
        return state != None && state != Destroyed;
    }
};

#endif // OVERLAYSTATE_H
