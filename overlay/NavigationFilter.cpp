#include "NavigationFilter.h"

#include <QEvent>
#include <QKeyEvent>

#include "Log.h"
#include "PageManager.h"
#include "PopupManager.h"

NavigationFilter::NavigationFilter(PageManager *pages, PopupManager *popups,
                                   QObject *parent)
    : QObject(parent)
    , m_pages(pages)
    , m_popups(popups)
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

    //==============================
    // 1. 有弹窗 → 先关视觉最上层的那个
    //==============================
    if (m_popups && m_popups->count() > 0)
    {
        qCInfo(logPopup) << "Back key -> close top popup";

        m_popups->closeTop();

        return true; // 吞掉，不再传给页面
    }

    //==============================
    // 2. 否则 back 页面
    //==============================
    if (m_pages)
    {
        qCInfo(logPageManager) << "Back key -> page back";

        m_pages->back();

        return true;
    }

    return false;
}
