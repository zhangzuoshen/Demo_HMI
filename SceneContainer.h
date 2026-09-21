#ifndef SCENECONTAINER_H
#define SCENECONTAINER_H

#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlIncubator>

#include "AppContext.h"
#include "AppInstance.h"

class PageManager;

/**
 * @brief 每个 Scene 的页面容器
 *
 * 一个 SceneContainer 对应一个独立 QQmlContext。
 * 每次切换页面都会创建新的 AppContext。
 */
class SceneContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* pageManager
               READ pageManager
               WRITE setPageManager
               NOTIFY pageManagerChanged)

public:

    explicit SceneContainer(QQuickItem *parent=nullptr);
    ~SceneContainer();

    QObject* pageManager() const;
    void setPageManager(QObject *mgr);

signals:
    void pageManagerChanged();

protected:
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;

private slots:
    void onCurrentAppChanged();

private:
    class SceneIncubator;

    void load(const AppInstance &instance);
    void unload();

    void finishCreate(QObject *object);
    void createError();

private:
    QObject *m_pageManager=nullptr;
    QQmlEngine *m_engine=nullptr;
    QQmlContext *m_context=nullptr;
    AppContext *m_appContext=nullptr;
    QQmlComponent *m_component=nullptr;
    SceneIncubator *m_incubator=nullptr;
    QQuickItem *m_rootItem=nullptr;
    AppInstance m_currentInstance;
};

#endif
