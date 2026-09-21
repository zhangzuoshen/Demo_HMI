#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QObject>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>

#include "AppInstance.h"
#include "WindowState.h"

class QQmlEngine;

class PageView : public QObject
{
    Q_OBJECT

    Q_PROPERTY(WindowState::State windowState
               READ windowState
               NOTIFY windowStateChanged)

public:
    explicit PageView(QQmlEngine *engine,
                      QObject *parent = nullptr);

    ~PageView();

    WindowState::State windowState() const;
    void setWindowState(WindowState::State s);

    bool create(const AppInstance &instance);

    void destroy();

    void attach(QQuickItem *parentItem);

    void detach();

    void resize(const QSizeF &size);

    bool isAttached() const;

    QQuickItem *rootItem() const;

    const AppInstance &instance() const;

signals:

    void windowStateChanged();

private:

    WindowState::State m_windowState =
            WindowState::Detached;

    QQmlEngine *m_engine = nullptr;

    QQmlContext *m_context = nullptr;

    QQmlComponent *m_component = nullptr;

    QQuickItem *m_rootItem = nullptr;

    bool m_attached = false;

    AppInstance m_instance;
};

#endif
