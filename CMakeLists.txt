cmake_minimum_required(VERSION 3.16)

project(Demo_HMI
    VERSION 1.0.0
    DESCRIPTION "Qt Quick HMI 框架实验工程"
    LANGUAGES CXX
)

# Qt 6 要求 C++17（原 Qt 5 工程使用 C++11）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

# Qt 6.3+：自动开启 AUTOMOC / AUTOUIC
qt_standard_project_setup()

qt_add_executable(Demo_HMI
    # --- 入口 ---
    main.cpp

    # --- 核心 C++ 层 ---
    AppContext.h            AppContext.cpp
    AppInstance.h           AppInstance.cpp
    AppRegistry.h           AppRegistry.cpp
    AppState.h
    ApplicationBootstrap.h  ApplicationBootstrap.cpp
    Log.h                   Log.cpp
    NavigationFilter.h      NavigationFilter.cpp
    OverlayContext.h        OverlayContext.cpp
    OverlayPriority.h
    OverlayState.h
    PageIncubator.h         PageIncubator.cpp
    PageManager.h           PageManager.cpp
    PageView.h              PageView.cpp
    PopupContext.h          PopupContext.cpp
    PopupIncubator.h        PopupIncubator.cpp
    PopupInstance.h
    PopupManager.h          PopupManager.cpp
    PopupRegistry.h         PopupRegistry.cpp
    PopupView.h             PopupView.cpp
    SceneContainer.h        SceneContainer.cpp
    WindowState.h
)

# QML 与资源：等价于原 apps/apps.qrc 的 prefix="/apps"
# 保持资源路径不变（:/apps/...），AppRegistry 无需改动
qt_add_resources(Demo_HMI "apps"
    PREFIX "/apps"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/apps"
    FILES
        apps/main.qml
        apps/Home/App.qml
        apps/Home/manifest.json
        apps/Media/App.qml
        apps/Media/manifest.json
        apps/Settings/App.qml
        apps/Settings/manifest.json
        apps/components/AppButton.qml
        apps/components/BasePage.qml
        apps/components/HeavyTestPage.qml
        apps/components/TitleBar.qml
)

target_link_libraries(Demo_HMI PRIVATE Qt6::Quick)

# 让已废弃的 Qt API 产生编译警告
target_compile_definitions(Demo_HMI PRIVATE QT_DEPRECATED_WARNINGS)
