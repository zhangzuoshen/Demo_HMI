#ifndef PAGEINCUBATOR_H
#define PAGEINCUBATOR_H

#include <QQmlIncubator>
#include <QPointer>

class PageView;

class PageIncubator : public QQmlIncubator
{
public:
    explicit PageIncubator(PageView *view);

protected:
    void statusChanged(Status status) override;
    void setInitialState(QObject *object) override;

private:
    QPointer<PageView> m_view;
};

#endif // PAGEINCUBATOR_H
