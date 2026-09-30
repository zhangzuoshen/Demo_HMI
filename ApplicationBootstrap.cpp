#include "ApplicationBootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "Log.h"
#include "NavigationFilter.h"
#include "OverlayState.h"
#include "PageManager.h"
#include "PopupContainer.h"
#include "PopupManager.h"
#include "SceneContainer.h"
#include "ToastContainer.h"
#include "ToastManager.h"
#include "WindowState.h"
#include "AppState.h"

//==============================
// QML 注册公共宏定义
//==============================
#define HMI_QML_URI        "HMI.Core"
#define HMI_QML_VER_MAJOR  1
#define HMI_QML_VER_MINOR  0

#define REG_HMI_ENUM(EnumClass, EnumQmlName) \
qmlRegisterUncreatableMetaObject(EnumClass::staticMetaObject, HMI_QML_URI, \
                                 HMI_QML_VER_MAJOR, HMI_QML_VER_MINOR, EnumQmlName, \
                                 EnumQmlName " is an enum only")

#define REG_HMI_TYPE(CppClass, QmlName) \
    qmlRegisterType<CppClass>(HMI_QML_URI, HMI_QML_VER_MAJOR, HMI_QML_VER_MINOR, QmlName)


bool ApplicationBootstrap::initialize(QQmlApplicationEngine &engine,
                                      AppRegistry &registry,
                                      PageManager &pageManager,
                                      PopupManager &popupManager,
                                      ToastManager &toastManager)
{
    Q_UNUSED(registry)

    //==============================
    // 注册 QML 类型
    //==============================
    REG_HMI_ENUM(WindowState, "WindowState");
    REG_HMI_ENUM(AppState, "AppState");
    REG_HMI_ENUM(OverlayState, "OverlayState");

    REG_HMI_TYPE(SceneContainer, "SceneContainer");
    REG_HMI_TYPE(PopupContainer, "PopupContainer");
    REG_HMI_TYPE(ToastContainer, "ToastContainer");

    engine.rootContext()->setContextProperty("PageManager", &pageManager);
    engine.rootContext()->setContextProperty("PopupManager", &popupManager);
    engine.rootContext()->setContextProperty("ToastManager", &toastManager);

    //==============================
    // 加载主界面
    //==============================
    engine.load(QUrl("qrc:/apps/main.qml"));

    auto rootObjects = engine.rootObjects();
    if (rootObjects.isEmpty())
    {
        qCCritical(logBootstrap) << "Failed to load main.qml";
        return false;
    }

    //==============================
    // Qt 5.12：绑定 QQmlIncubationController
    //==============================
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObjects.first());

    if (window)
    {
        engine.setIncubationController(window->incubationController());

        qCInfo(logBootstrap) << "QQmlIncubationController attached.";

        //==============================
        // 返回键路由
        //==============================
        // 不挂在 QML 的 Keys handler 上：那种写法依赖焦点树，
        // 弹窗抢焦点后按键就不再经过页面那一层，会误触发页面返回。
        auto *filter =
            new NavigationFilter(&pageManager, &popupManager, window);

        window->installEventFilter(filter);

        qCInfo(logBootstrap) << "Navigation filter installed.";
    }
    else
    {
        qCWarning(logBootstrap) << "Root object is not QQuickWindow.";
    }

    return true;
}
