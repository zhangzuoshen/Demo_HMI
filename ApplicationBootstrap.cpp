#include "ApplicationBootstrap.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Log.h"
#include "PageManager.h"
#include "SceneContainer.h"
#include "WindowState.h"

bool ApplicationBootstrap::initialize(
        QQmlApplicationEngine &engine,
        AppRegistry &registry,
        PageManager &pageManager)
{
    Q_UNUSED(registry)

    //==============================
    // 注册 QML 类型
    //==============================
    qmlRegisterUncreatableMetaObject(
                WindowState::staticMetaObject,
                "HMI.Core",     // QML模块
                1, 0,
                "WindowState",  // QML类型名
                "WindowState is an enum only");

    qmlRegisterUncreatableMetaObject(
                AppState::staticMetaObject,
                "HMI.Core",      // QML模块
                1, 0,
                "AppState",      // QML类型名
                "AppState is an enum only");


    qmlRegisterType<SceneContainer>(
                "HMI.Core",
                1,
                0,
                "SceneContainer");

    engine.rootContext()->setContextProperty(
                "PageManager",
                &pageManager);

    engine.load(QUrl("qrc:/apps/main.qml"));

    if(engine.rootObjects().isEmpty())
    {
        qCCritical(logBootstrap)
                << "Failed to load main.qml";

        return -1;
    }

    return !engine.rootObjects().isEmpty();
}
