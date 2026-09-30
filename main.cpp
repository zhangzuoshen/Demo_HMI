#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>
#include <QSurfaceFormat>

#include "Log.h"
#include "AppRegistry.h"
#include "PageManager.h"
#include "PopupManager.h"
#include "PopupRegistry.h"
#include "ToastManager.h"
#include "ToastRegistry.h"
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
    // 注意：Qt 6 的 scenegraph 要求 OpenGL ES 3.0+ 或 OpenGL 3.3 core，
    //       Qt 5 时代的 OpenGL ES 2.0 已不再受支持。
    QSurfaceFormat format;

    // 嵌入式（EGLFS / LinuxFB）走 OpenGL ES 3.0，桌面保持默认
    const QString platform = app.platformName();

    if (platform == QStringLiteral("eglfs")
        || platform == QStringLiteral("linuxfb"))
    {
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setVersion(3, 0);
    }

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
    // Popup Registry / Manager
    //==============================
    // 没有注册任何弹窗不算错误，只记录日志
    PopupRegistry popupRegistry;

    if (!popupRegistry.loadPopups(":/apps"))
        qCInfo(logMain) << "No popup registered.";

    PopupManager popupManager(&popupRegistry, &pageManager);

    //==============================
    // QML Engine
    //==============================
    QQmlApplicationEngine engine;

    //==============================
    // Toast Registry / Manager
    //==============================
    // Toast 走同步创建，构造时就拿着 QQmlEngine，所以必须晚于 engine
    ToastRegistry toastRegistry;

    if (!toastRegistry.loadToasts(":/apps"))
        qCInfo(logMain) << "No toast registered.";

    ToastManager toastManager(&toastRegistry, &engine, &pageManager);

    if (!ApplicationBootstrap::initialize(engine, registry, pageManager,
                                        popupManager, toastManager))
    {
        qCCritical(logMain) << "Failed to load Main.qml";

        return -1;
    }

    // preload 的 Toast 在此完成 QML 编译，首次 show 不再有编译开销
    toastRegistry.preloadAll(&engine);

    qCInfo(logMain) << "HMI initialized successfully.";

    //==============================
    // 启动默认应用（Home）
    // 放在 Main.qml 加载完成之后，
    // 保证 SceneContainer 已连接信号。
    //==============================
    pageManager.launch("home");

    return app.exec();
}
