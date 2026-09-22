#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "AppInstance.h"
#include "WindowState.h"

class PageIncubator;

class PageView : public QObject
{
    Q_OBJECT

public:
    explicit PageView(QQmlEngine *engine,
                      QObject *parent = nullptr);
    ~PageView() override;

    bool create(const AppInstance &instance);
    void destroy();

    bool isReady() const;

    void attach(QQuickItem *parent);
    void detach();

    bool isAttached() const;
    void resize(const QSizeF &size);

    const AppInstance &instance() const;

    WindowState::State windowState() const;

    void onIncubationReady();
    void onIncubationError(const QList<QQmlError> &errors);

signals:
    void incubationReady(quint64 instanceId);
    void incubationFailed(quint64 instanceId);

private:
    void setWindowState(WindowState::State state);

private:
    QQmlEngine *m_engine = nullptr;
    QQmlContext *m_context = nullptr;
    QQmlComponent *m_component = nullptr;
    PageIncubator *m_incubator = nullptr;
    QQuickItem *m_rootItem = nullptr;

    AppInstance m_instance;

    bool m_ready = false;
    bool m_attached = false;

    // 新增：销毁保护，防止孵化完成回调访问已释放对象
    bool m_destroying = false;

    WindowState::State m_windowState = WindowState::Detached;
};

#endif
