#include "PageIncubator.h"

#include "PageView.h"

PageIncubator::PageIncubator(PageView *view)
    : QQmlIncubator(QQmlIncubator::AsynchronousIfNested)
    , m_view(view)
{
}

void PageIncubator::statusChanged(Status status)
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

void PageIncubator::setInitialState(QObject *object)
{
    if (!m_view)
        return;

    object->setParent(m_view); // 生命周期交给 PageView
}
