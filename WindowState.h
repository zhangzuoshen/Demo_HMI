#ifndef WINDOWSTATE_H
#define WINDOWSTATE_H

#include <QObject>
#include <QString>

class WindowState
{
    Q_GADGET

public:
    enum State
    {
        // 未创建或已从窗口树分离
        Detached = 0,

        // QQmlIncubator 正在异步创建
        Creating,

        // QML 已创建完成，但尚未 attach 到 SceneContainer
        Ready,

        // 已 attach 到 SceneContainer
        Attached,

        // 已显示（预留给转场动画结束）
        Visible,

        // 正在执行窗口切换动画
        Transition
    };
    Q_ENUM(State)

    static QString toString(State state)
    {
        switch (state)
        {
        case Detached:   return QStringLiteral("Detached");
        case Creating:   return QStringLiteral("Creating");
        case Ready:      return QStringLiteral("Ready");
        case Attached:   return QStringLiteral("Attached");
        case Visible:    return QStringLiteral("Visible");
        case Transition: return QStringLiteral("Transition");
        }

        return QStringLiteral("Unknown");
    }

    // 是否已经加入窗口树
    static bool isAttached(State state)
    {
        return state == Attached ||
               state == Visible ||
               state == Transition;
    }

    // 是否已经完成 QML 创建
    static bool isReady(State state)
    {
        return state == Ready ||
               state == Attached ||
               state == Visible ||
               state == Transition;
    }

    // 是否仍然存活（包括后台缓存）
    static bool isAlive(State state)
    {
        return state != Detached;
    }
};

#endif // WINDOWSTATE_H
