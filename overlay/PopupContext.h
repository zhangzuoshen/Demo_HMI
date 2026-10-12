#ifndef POPUPCONTEXT_H
#define POPUPCONTEXT_H

#include <QVariantMap>

#include "OverlayContext.h"

/**
 * @brief 暴露给 QML 的弹窗上下文（context property 名 "PopupContext"）
 *
 * state / notifyClosed() / closed() / stateChanged() 由基类 OverlayContext 提供。
 *
 * 关键约定：close() 只发 closeRequested，不立即销毁。
 * 由 PopupManager 置 Closing → QML 播退场动画 → notifyClosed() → 才真正销毁。
 */
class PopupContext : public OverlayContext
{
    Q_OBJECT

    Q_PROPERTY(QString popupId READ popupId CONSTANT)

    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(bool modal READ isModal CONSTANT)

    Q_PROPERTY(bool dim READ dim CONSTANT)

    Q_PROPERTY(QVariantMap args READ args NOTIFY argsChanged)

    Q_PROPERTY(QVariant result READ result NOTIFY resultChanged)

public:
    explicit PopupContext(QObject *parent = nullptr);

    QString popupId() const;
    QString name() const;
    bool isModal() const;
    bool dim() const;
    QVariantMap args() const;
    QVariant result() const;

    // QML 侧调用：请求关闭。不立即销毁，只置 Closing 并等退场动画
    Q_INVOKABLE void close(const QVariant &result = QVariant());

    // 等价于 close(false)
    Q_INVOKABLE void dismiss();

    // 便捷取参数，避免 QML 里 PopupContext.args.title 取到 undefined
    Q_INVOKABLE QVariant arg(const QString &key,
                             const QVariant &defaultValue = QVariant()) const;

    // C++ 内部
    void setPopupId(const QString &id);
    void setName(const QString &name);
    void setModal(bool modal);
    void setDim(bool dim);
    void setArgs(const QVariantMap &args);
    void setResult(const QVariant &result);

signals:
    void closeRequested(const QVariant &result);

    void argsChanged();

    void resultChanged();

private:
    QString m_popupId;
    QString m_name;
    bool m_modal = true;
    bool m_dim = true;
    QVariantMap m_args;
    QVariant m_result;
};

#endif // POPUPCONTEXT_H
