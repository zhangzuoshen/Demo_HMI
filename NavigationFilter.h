#ifndef NAVIGATIONFILTER_H
#define NAVIGATIONFILTER_H

#include <QObject>
#include <QPointer>

class PageManager;

/**
 * @brief 全局返回键路由
 *
 * 装在 QQuickWindow 上做事件过滤，而不是挂 QML 的 Keys handler。
 * 原因：Keys handler 依赖焦点树，弹窗一抢焦点，按键就沿弹窗的父链
 * 上传（→ PopupContainer → popupScene → Window），根本不经过页面那层的
 * handler，导致"弹窗打开时按返回键把页面 back 掉"。
 *
 * 事件过滤器在窗口级别拦截，与焦点无关，方向盘硬按键也能覆盖。
 */
class NavigationFilter : public QObject
{
    Q_OBJECT

public:
    explicit NavigationFilter(PageManager *pages, QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QPointer<PageManager> m_pages;
};

#endif // NAVIGATIONFILTER_H
