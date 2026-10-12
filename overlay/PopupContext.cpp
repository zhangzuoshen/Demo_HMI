#include "PopupContext.h"

PopupContext::PopupContext(QObject *parent)
    : OverlayContext(parent)
{
}

QString PopupContext::popupId() const
{
    return m_popupId;
}

QString PopupContext::name() const
{
    return m_name;
}

bool PopupContext::isModal() const
{
    return m_modal;
}

bool PopupContext::dim() const
{
    return m_dim;
}

QVariantMap PopupContext::args() const
{
    return m_args;
}

QVariant PopupContext::result() const
{
    return m_result;
}

void PopupContext::close(const QVariant &result)
{
    // 只发请求，销毁由 PopupManager 在退场动画结束后执行
    emit closeRequested(result);
}

void PopupContext::dismiss()
{
    close(QVariant(false));
}

QVariant PopupContext::arg(const QString &key, const QVariant &defaultValue) const
{
    return m_args.value(key, defaultValue);
}

void PopupContext::setPopupId(const QString &id)
{
    m_popupId = id;
}

void PopupContext::setName(const QString &name)
{
    m_name = name;
}

void PopupContext::setModal(bool modal)
{
    m_modal = modal;
}

void PopupContext::setDim(bool dim)
{
    m_dim = dim;
}

void PopupContext::setArgs(const QVariantMap &args)
{
    if (m_args == args)
        return;

    m_args = args;

    emit argsChanged();
}

void PopupContext::setResult(const QVariant &result)
{
    if (m_result == result)
        return;

    m_result = result;

    emit resultChanged();
}
