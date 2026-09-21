#include "SceneContainer.h"

#include <QQmlEngine>

#include "AppInstance.h"
#include "PageManager.h"
#include "PageView.h"
#include "Log.h"

SceneContainer::SceneContainer(QQuickItem *parent)
    : QQuickItem(parent)
{
}

SceneContainer::~SceneContainer()
{
    qDeleteAll(m_cachedViews);
    m_cachedViews.clear();
}

QObject* SceneContainer::pageManager() const
{
    return m_pageManager;
}

void SceneContainer::setPageManager(QObject *mgr)
{
    if(m_pageManager==mgr)
        return;

    if(m_pageManager)
        disconnect(m_pageManager,nullptr,this,nullptr);

    m_pageManager=mgr;

    if(m_pageManager)
    {
        connect(m_pageManager,
                SIGNAL(currentChanged()),
                this,
                SLOT(onCurrentChanged()));

        onCurrentChanged();
    }

    emit pageManagerChanged();
}

// 查找缓存
PageView *SceneContainer::findView(quint64 instanceId)
{
    return m_cachedViews.value(instanceId,nullptr);
}

// 创建缓存
PageView *SceneContainer::createView(
        const AppInstance &instance)
{
    PageView *view=
            new PageView(qmlEngine(this),this);

    if(!view->create(instance))
    {
        delete view;
        return nullptr;
    }

    view->resize(QSizeF(width(),height()));

    m_cachedViews.insert(instance.instanceId,view);

    return view;
}

// 页面切换（核心）
void SceneContainer::onCurrentChanged()
{
    auto mgr=
            qobject_cast<PageManager*>(m_pageManager);

    if(!mgr)
        return;

    AppInstance instance=
            mgr->currentApp();

    if(instance.instanceId==0)
        return;

    qCInfo(logScene)
            << "Load:"
            << instance.info.appId
            << "#"
            << instance.instanceId;

    PageView *next=
            findView(instance.instanceId);

    bool firstCreate=false;

    if(!next)
    {
        next=createView(instance);

        if(!next)
            return;

        firstCreate=true;
    }

    if(m_frontView &&
       m_frontView!=next)
    {
        const AppInstance &old=
                m_frontView->instance();

        if(old.info.keepAlive)
        {
            qCInfo(logScene)
                    << "Detach:"
                    << old.info.appId;

            m_frontView->detach();
        }
        else
        {
            qCInfo(logScene)
                    << "Destroy:"
                    << old.info.appId;

            m_cachedViews.remove(old.instanceId);

            delete m_frontView;
        }
    }

    m_frontView=next;

    m_frontView->attach(this);

    m_frontView->resize(QSizeF(width(),height()));

    if(firstCreate)
    {
        qCInfo(logScene)
                << "Notify pageReady:"
                << instance.instanceId;

        mgr->pageReady(instance.instanceId);
    }
}

// 自动适配大小
void SceneContainer::geometryChanged(
        const QRectF &newGeometry,
        const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(
                newGeometry,
                oldGeometry);

    for(auto view:m_cachedViews)
        view->resize(newGeometry.size());
}
