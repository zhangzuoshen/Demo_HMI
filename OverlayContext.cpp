#include "OverlayContext.h"

OverlayContext::OverlayContext(QObject *parent)
    : QObject(parent)
{
}

OverlayState::State OverlayContext::state() const
{
    return m_state;
}

void OverlayContext::notifyClosed()
{
    emit closed();
}

void OverlayContext::setState(OverlayState::State state)
{
    if (m_state == state)
        return;

    m_state = state;

    emit stateChanged();
}
