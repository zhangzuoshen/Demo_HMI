#include "ToastContext.h"

ToastContext::ToastContext(QObject *parent)
    : OverlayContext(parent)
{
}

QString ToastContext::toastId() const
{
    return m_toastId;
}

QString ToastContext::name() const
{
    return m_name;
}

QString ToastContext::message() const
{
    return m_message;
}

int ToastContext::duration() const
{
    return m_duration;
}

QVariantMap ToastContext::args() const
{
    return m_args;
}

void ToastContext::setToastId(const QString &id)
{
    m_toastId = id;
}

void ToastContext::setName(const QString &name)
{
    m_name = name;
}

void ToastContext::setMessage(const QString &message)
{
    if (m_message == message)
        return;

    m_message = message;

    emit messageChanged();
}

void ToastContext::setDuration(int duration)
{
    if (m_duration == duration)
        return;

    m_duration = duration;

    emit durationChanged();
}

void ToastContext::setArgs(const QVariantMap &args)
{
    m_args = args;

    emit argsChanged();
}

void ToastContext::dismiss()
{
    emit dismissRequested();
}

void ToastContext::triggerAction(const QString &name, const QVariant &data)
{
    emit actionTriggered(name, data);
}

QVariant ToastContext::arg(const QString &key,
                           const QVariant &defaultValue) const
{
    return m_args.value(key, defaultValue);
}
