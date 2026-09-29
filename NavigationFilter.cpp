#include "NavigationFilter.h"

#include <QEvent>
#include <QKeyEvent>

#include "Log.h"
#include "PageManager.h"

NavigationFilter::NavigationFilter(PageManager *pages, QObject *parent)
    : QObject(parent)
    , m_pages(pages)
{
}

bool NavigationFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if (event->type() != QEvent::KeyRelease)
        return false;

    auto *key = static_cast<QKeyEvent *>(event);

    if (key->isAutoRepeat())
        return false;

    if (key->key() != Qt::Key_Back && key->key() != Qt::Key_Escape)
        return false;

    // 页面上没有焦点项时，事件本就无人消费，此处统一接管
    if (!m_pages)
        return false;

    qCInfo(logPageManager) << "Back key";

    m_pages->back();

    return true;
}
