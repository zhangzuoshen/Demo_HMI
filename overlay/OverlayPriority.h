#ifndef OVERLAYPRIORITY_H
#define OVERLAYPRIORITY_H

#include <QObject>
#include <QString>

/**
 * @brief Dialog 与 Toast 共用的优先级量级
 *
 * Dialog 与 Toast 是两个独立系统，但可能会互相比较优先级（例如
 * 判断"当前屏幕上最紧急的是什么"）。若各自定义一套数值，
 * 跨系统比较就会失效，所以统一在这里定义。
 *
 * manifest 里直接写数字（"priority": 80），日志打印时用 toString() 转名字。
 */
class OverlayPriority
{
    Q_GADGET

public:
    enum Level
    {
        Low = 10,      // 可选提示：新版本可用、状态更新
        Normal = 50,   // 默认：业务确认、普通提示
        High = 80,     // 安全确认、操作失败
        Critical = 100 // 系统致命警告、强制升级
    };
    Q_ENUM(Level)

    static QString toString(int priority)
    {
        switch (priority)
        {
        case Low: return QStringLiteral("Low(10)");

        case Normal: return QStringLiteral("Normal(50)");

        case High: return QStringLiteral("High(80)");

        case Critical: return QStringLiteral("Critical(100)");
        }

        return QStringLiteral("Custom(%1)").arg(priority);
    }
};

#endif // OVERLAYPRIORITY_H
