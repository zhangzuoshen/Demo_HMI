#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

#include "Log.h"
#include "AppRegistry.h"
#include "PageManager.h"
#include "ApplicationBootstrap.h"

int main(int argc, char *argv[])
{
    //==============================
    // Qt Application
    //==============================
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("Demo_HMI");
    QCoreApplication::setApplicationVersion("1.0.0");

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

    qCInfo(logMain) << "Application started";

    //==============================
    // App Registry
    //==============================
    AppRegistry registry;

    if (!registry.loadApps(":/apps"))
    {
        qCCritical(logMain) << "No application found.";

        return -1;
    }

    //==============================
    // Page Manager
    //==============================
    PageManager pageManager(&registry);

    //==============================
    // QML Engine
    //==============================
    QQmlApplicationEngine engine;

    if (!ApplicationBootstrap::initialize(engine, registry, pageManager))
    {
        qCCritical(logMain) << "Failed to load Main.qml";

        return -1;
    }

    //==============================
    // 启动默认应用（Home）
    // 放在 Main.qml 加载完成之后，
    // 保证 SceneContainer 已连接信号。
    //==============================
    pageManager.launch("home");

    qCInfo(logMain) << "HMI initialized successfully.";

    return app.exec();
}
