#ifndef POPUPINCUBATOR_H
#define POPUPINCUBATOR_H

#include <QQmlIncubator>
#include <QPointer>

class PopupView;

/**
 * @brief 弹窗的异步孵化器
 *
 * 与 PageIncubator 同构，但不复用它 —— 避免 PageIncubator 持有 PageView 类型。
 */
class PopupIncubator : public QQmlIncubator
{
public:
    explicit PopupIncubator(PopupView *view, IncubationMode mode = Asynchronous);

protected:
    void statusChanged(Status status) override;
    void setInitialState(QObject *object) override;

private:
    QPointer<PopupView> m_view;
};

#endif // POPUPINCUBATOR_H
