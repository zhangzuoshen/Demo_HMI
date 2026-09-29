#include "ApplicationBootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "Log.h"
#include "NavigationFilter.h"
#include "PageManager.h"
#include "SceneContainer.h"
#include "WindowState.h"
#include "AppState.h"

bool ApplicationBootstrap::initialize(QQmlApplicationEngine &engine,
                                      AppRegistry &registry,
                                      PageManager &pageManager)
{
    Q_UNUSED(registry)

    //==============================
    // 注册 QML 类型
    //==============================
    qmlRegisterUncreatableMetaObject(WindowState::staticMetaObject, "HMI.Core",
                                     1, 0, "WindowState",
                                     "WindowState is an enum only");

    qmlRegisterUncreatableMetaObject(AppState::staticMetaObject, "HMI.Core", 1,
                                     0, "AppState", "AppState is an enum only");

    qmlRegisterType<SceneContainer>("HMI.Core", 1, 0, "SceneContainer");

    engine.rootContext()->setContextProperty("PageManager", &pageManager);

    //==============================
    // 加载主界面
    //==============================
    engine.load(QUrl("qrc:/apps/main.qml"));

    if (engine.rootObjects().isEmpty())
    {
        qCCritical(logBootstrap) << "Failed to load main.qml";
        return false;
    }

    //==============================
    // Qt 5.12：绑定 QQmlIncubationController
    //==============================
    QQuickWindow *window =
        qobject_cast<QQuickWindow *>(engine.rootObjects().first());

    if (window)
    {
        engine.setIncubationController(window->incubationController());

        qCInfo(logBootstrap) << "QQmlIncubationController attached.";

        //==============================
        // 返回键路由
        //==============================
        // 不挂在 QML 的 Keys handler 上：那种写法依赖焦点树，
        // 弹窗抢焦点后按键就不再经过页面那一层，会误触发页面返回。
        auto *filter = new NavigationFilter(&pageManager, window);

        window->installEventFilter(filter);

        qCInfo(logBootstrap) << "Navigation filter installed.";
    }
    else
    {
        qCWarning(logBootstrap) << "Root object is not QQuickWindow.";
    }

    return true;
}
