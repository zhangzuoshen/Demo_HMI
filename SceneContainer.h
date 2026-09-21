#ifndef SCENECONTAINER_H
#define SCENECONTAINER_H

#include <QQuickItem>
#include <QScopedPointer>

class PageManager;
class PageView;

class SceneContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject *pageManager
               READ pageManager
               WRITE setPageManager
               NOTIFY pageManagerChanged)

public:

    explicit SceneContainer(QQuickItem *parent = nullptr);

    ~SceneContainer() override;

    QObject *pageManager() const;

    void setPageManager(QObject *mgr);

signals:

    void pageManagerChanged();

protected:

    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;

private slots:

    void onCurrentChanged();

private:

    QObject *m_pageManager = nullptr;

    QScopedPointer<PageView> m_currentView;
};

#endif
