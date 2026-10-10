# Qt 6.9 硬性要求 CMake >= 3.21（Qt6Config.cmake 内部检查），
# 官方标注的支持下限 QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_USING_QT 是 3.22
cmake_minimum_required(VERSION 3.22)

project(Demo_HMI
    VERSION 1.0.0
    DESCRIPTION "Qt Quick HMI 框架实验工程"
    LANGUAGES CXX
)

# Qt 6 要求 C++17（原 Qt 5 工程使用 C++11）。
# Qt 6.9 的 ABI 基线仍然是 C++17，语言标准不跟着 SDK 一起升，
# 保持与 SDK 预编译库一致，避免无收益的 ABI 风险。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 开发机 SDK：~/SDK/Qt6/6.9.3/gcc_64（CMake 3.30.5）
find_package(Qt6 6.9 REQUIRED COMPONENTS Quick)

message(STATUS "Demo_HMI: Qt ${Qt6_VERSION} / CMake ${CMAKE_VERSION} / C++${CMAKE_CXX_STANDARD}")

# 注：Qt 6.9 的 QT_QML_GENERATE_QMLLS_INI（给 Qt Creator 的 QML 语言服务器用）
# 只在 qt_add_qml_module 的路径上生效。本工程走 qt_add_resources，
# 为的是保持 :/apps/... 资源路径不变（AppRegistry / PopupRegistry / ToastRegistry
# 都按这个前缀扫描），所以这里不开。将来若迁移到 QML 模块，再打开它。

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
    PopupContainer.h        PopupContainer.cpp
    PopupContext.h          PopupContext.cpp
    PopupIncubator.h        PopupIncubator.cpp
    PopupInstance.h
    PopupManager.h          PopupManager.cpp
    PopupRegistry.h         PopupRegistry.cpp
    PopupView.h             PopupView.cpp
    SceneContainer.h        SceneContainer.cpp
    ToastContainer.h        ToastContainer.cpp
    ToastContext.h          ToastContext.cpp
    ToastManager.h          ToastManager.cpp
    ToastRegistry.h         ToastRegistry.cpp
    WindowState.h
)

# QML 与资源：等价于原 apps/apps.qrc 的 prefix="/apps"
# 保持资源路径不变（:/apps/...），AppRegistry 无需改动
# ---------------------------------------------------------------
# QML 模块：apps/ 下的 QML 全部编译进 HMI.Apps（Qt 6 推荐做法）
#   - 同一模块内类型互相可见，不再需要 import "../../../components"
#   - 资源前缀由 URI 决定：<RESOURCE_PREFIX>/<URI 点转斜杠>/<文件相对路径>
#   - C++ 侧的扫描根路径由 HMI_APPS_QML_ROOT 常量给出，与这里保持一致
# ---------------------------------------------------------------
set(HMI_APPS_URI "HMI.Apps")
string(REPLACE "." "/" HMI_APPS_URI_PATH "${HMI_APPS_URI}")

# QTP0004：Qt 6.9 要求"模块内每个含 .qml 的子目录都有自己的 qmldir"。
# 本工程按业务结构组织（apps/<app>/popups/<id>/...），三个 registry 都靠
# 这套目录扫描，拆成子模块不现实，所以显式保留旧行为：
# 一模块容纳全部，子目录里的 QML 由模块根 qmldir 的带路径条目声明，
# 文件里用 import HMI.Apps 显式导入自己的模块。
# 注意：Qt 提示 OLD 行为会在未来大版本移除。届时的两个出路：
#   a) 给每个含 .qml 的目录补 qmldir（拆成子模块）；
#   b) 调整目录布局，让 QML 直接落在模块根下。
qt_policy(SET QTP0004 OLD)

# 走 QML 模块后这个开关才真正生效：构建时生成 qmlls.ini，
# Qt Creator 的 QML 语言服务器才能正确索引本工程
set(QT_QML_GENERATE_QMLLS_INI ON)

qt_add_qml_module(Demo_HMI
    URI "${HMI_APPS_URI}"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"

    # --- 大写开头：注册为模块类型，模块内可直接引用 ---
    QML_FILES
        apps/components/AppButton.qml
        apps/components/BasePage.qml
        apps/components/BasePopup.qml
        apps/components/BaseToast.qml
        apps/components/HeavyTestPage.qml
        apps/components/TitleBar.qml
        apps/Home/HomeApp.qml
        apps/Media/MediaApp.qml
        apps/Media/popups/TrackInfo/TrackInfo.qml
        apps/Media/toasts/PlayFailed/PlayFailed.qml
        apps/Media/toasts/VolumeChanged/VolumeChanged.qml
        apps/Settings/SettingsApp.qml
        apps/System/popups/HintTip/HintTip.qml
        apps/System/popups/ThermalWarning/ThermalWarning.qml
        apps/System/toasts/LowStorage/LowStorage.qml
        apps/System/toasts/UsbConnected/UsbConnected.qml

    # --- 入口（小写开头，不作为类型）、清单、JS 辅助：仅打包不注册 ---
    RESOURCES
        apps/main.qml
        apps/components/PopupHelper.js
        apps/Home/manifest.json
        apps/Media/manifest.json
        apps/Media/popups/TrackInfo/manifest.json
        apps/Media/toasts/PlayFailed/manifest.json
        apps/Media/toasts/VolumeChanged/manifest.json
        apps/Settings/manifest.json
        apps/System/manifest.json
        apps/System/popups/HintTip/manifest.json
        apps/System/popups/ThermalWarning/manifest.json
        apps/System/toasts/LowStorage/manifest.json
        apps/System/toasts/UsbConnected/manifest.json
)

target_link_libraries(Demo_HMI PRIVATE Qt6::Quick)

# 让已废弃的 Qt API 产生编译警告
target_compile_definitions(Demo_HMI PRIVATE
    QT_DEPRECATED_WARNINGS
    # C++ 侧扫描 apps 的根路径，唯一真相源（随 URI / 前缀变化）
    HMI_APPS_QML_ROOT=":/qt/qml/${HMI_APPS_URI_PATH}/apps"
)

# 部署：cmake --install 时自动带上 Qt 运行库（打包到目标机 / 板子调试用）
include(GNUInstallDirs)

install(TARGETS Demo_HMI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE  DESTINATION .
)

qt_generate_deploy_app_script(
    TARGET Demo_HMI
    OUTPUT_SCRIPT hmi_deploy_script
)
install(SCRIPT ${hmi_deploy_script})
