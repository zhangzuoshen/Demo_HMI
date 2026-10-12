#include "PopupIncubator.h"

#include "PopupView.h"

PopupIncubator::PopupIncubator(PopupView *view, IncubationMode mode)
    : QQmlIncubator(mode)
    , m_view(view)
{
}

void PopupIncubator::statusChanged(Status status)
{
    if (!m_view)
        return;

    switch (status)
    {
    case Ready: m_view->onIncubationReady(); break;
    case Error: m_view->onIncubationError(errors()); break;
    default: break;
    }
}

void PopupIncubator::setInitialState(QObject *object)
{
    if (!m_view)
        return;

    object->setParent(m_view); // 生命周期交给 PopupView
}
