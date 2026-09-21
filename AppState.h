#ifndef APPSTATE_H
#define APPSTATE_H

#include <QObject>
#include <QString>

class AppState
{
    Q_GADGET

public:
    enum State
    {
        None = 0,

        // 实例已创建（QML 尚未 Ready）
        Created,

        // QML 已完成初始化
        Ready,

        // 前台运行
        Foreground,

        // 后台运行（KeepAlive）
        Background,

        // 被其他页面覆盖（Overlay、CarPlay 等）
        Covered,

        // 挂起（系统节能）
        Suspended,

        // 已销毁
        Destroyed
    };
    Q_ENUM(State)

    static QString toString(State state)
    {
        switch (state)
        {
        case None:
            return QStringLiteral("AppState::None");

        case Created:
            return QStringLiteral("AppState::Created");

        case Ready:
            return QStringLiteral("AppState::Ready");

        case Foreground:
            return QStringLiteral("AppState::Foreground");

        case Background:
            return QStringLiteral("AppState::Background");

        case Covered:
            return QStringLiteral("AppState::Covered");

        case Suspended:
            return QStringLiteral("AppState::Suspended");

        case Destroyed:
            return QStringLiteral("AppState::Destroyed");
        }

        return QStringLiteral("AppState::Unknown");
    }

    static bool isActive(State state)
    {
        return state == Foreground || state == Covered;
    }

    static bool isAlive(State state)
    {
        return state != None && state != Destroyed;
    }
};

#endif // APPSTATE_H
