#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

#include "Log.h"
#include "AppRegistry.h"
#include "PageManager.h"
#include "SceneContainer.h"

int main(int argc, char *argv[])
{
    //==============================
    // Qt Application
    //==============================
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("DemoHMI");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("Demo");

    //==============================
    // OpenGL（EGFS 推荐）
    //==============================
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(2, 0);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    //==============================
    // 日志系统
    //==============================
    initLogSystem();

    qCInfo(logPageManager)
            << "Application started";

    //==============================
    // App Registry
    //==============================
    AppRegistry registry;

    if(!registry.loadApps(":/apps"))
    {
        qCCritical(logPageManager)
                << "No application found.";

        return -1;
    }

    //==============================
    // Page Manager
    //==============================
    PageManager pageManager(&registry);

    //==============================
    // 注册 QML 类型
    //==============================
    qmlRegisterType<SceneContainer>(
                "HMI.Scene",
                1,
                0,
                "SceneContainer");

    //==============================
    // QML Engine
    //==============================
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(
                "PageManager",
                &pageManager);

    engine.load(QUrl("qrc:/apps/main.qml"));

    if(engine.rootObjects().isEmpty())
    {
        qCCritical(logPageManager)
                << "Failed to load main.qml";

        return -1;
    }

    //==============================
    // 启动默认应用（Home）
    // 放在 Main.qml 加载完成之后，
    // 保证 SceneContainer 已连接信号。
    //==============================
    pageManager.launchApp("home");

    qCInfo(logPageManager)
            << "HMI initialized successfully.";

    return app.exec();
}
