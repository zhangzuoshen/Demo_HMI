#ifndef TOASTCONTEXT_H
#define TOASTCONTEXT_H

#include <QString>
#include <QVariantMap>

#include "OverlayContext.h"

/**
 * @brief 暴露给 QML 的 Toast 上下文（context property 名 "ToastContext"）
 *
 * state / notifyClosed() / closed() / stateChanged() 由基类 OverlayContext 提供。
 *
 * 与 PopupContext 的关键差别：**没有 result**。
 * Toast 不返回值，只有可选的 action 回调（triggerAction）。
 */
class ToastContext : public OverlayContext
{
    Q_OBJECT

    Q_PROPERTY(QString toastId READ toastId CONSTANT)

    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)

    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)

    Q_PROPERTY(QVariantMap args READ args NOTIFY argsChanged)

public:
    explicit ToastContext(QObject *parent = nullptr);

    QString toastId() const;
    QString name() const;
    QString message() const;
    int duration() const;
    QVariantMap args() const;

    void setToastId(const QString &id);
    void setName(const QString &name);
    void setMessage(const QString &message);

    // collapse 命中时可热更新文案与参数，QML 侧用绑定而非缓存才拿得到新值
    void setArgs(const QVariantMap &args);
    void setDuration(int duration);

    // QML 调用：手动关闭（sticky Toast 的唯一关闭途径）
    Q_INVOKABLE void dismiss();

    // QML 调用：Toast 上的按钮（如"撤销" / "重试"）
    Q_INVOKABLE void triggerAction(const QString &name,
                                   const QVariant &data = QVariant());

    Q_INVOKABLE QVariant arg(const QString &key,
                             const QVariant &defaultValue = QVariant()) const;

signals:
    void dismissRequested();

    void actionTriggered(const QString &name, const QVariant &data);

    void messageChanged();

    void durationChanged();

    void argsChanged();

private:
    QString m_toastId;
    QString m_name;
    QString m_message;
    int m_duration = 3000;
    QVariantMap m_args;
};

#endif // TOASTCONTEXT_H
