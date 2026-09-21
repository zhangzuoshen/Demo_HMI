#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QObject>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>

#include "AppInstance.h"

class QQmlEngine;

class PageView : public QObject
{
    Q_OBJECT

public:
    explicit PageView(QQmlEngine *engine,
                      QObject *parent = nullptr);

    ~PageView();

    bool create(const AppInstance &instance);

    void destroy();

    void attach(QQuickItem *parentItem);

    void detach();

    void resize(const QSizeF &size);

    bool isAttached() const;

    QQuickItem *rootItem() const;

    const AppInstance &instance() const;

private:

    QQmlEngine *m_engine = nullptr;

    QQmlContext *m_context = nullptr;

    QQmlComponent *m_component = nullptr;

    QQuickItem *m_rootItem = nullptr;

    bool m_attached = false;

    AppInstance m_instance;
};

#endif
