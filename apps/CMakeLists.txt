# ============================================================================
# apps：QML 模块树
#
#   HMI.Apps            <- 本文件：入口 main.qml + 共享组件（架构 owner 维护）
#   HMI.Apps.Home       <- Home/         （各 app owner 维护自己的 CMakeLists）
#   HMI.Apps.Media      <- Media/
#   HMI.Apps.Settings   <- Settings/
#   HMI.Apps.System     <- System/       （只有弹窗 / Toast，无入口页面）
#
# 资源布局：<RESOURCE_PREFIX>/<URI 点转斜杠>/<文件相对模块 base 的路径>
#   main.qml            -> qrc:/qt/qml/HMI/Apps/main.qml
#   components/BasePage.qml -> qrc:/qt/qml/HMI/Apps/components/BasePage.qml
#   Home/HomeApp.qml    -> qrc:/qt/qml/HMI/Apps/Home/HomeApp.qml
#
# 因此 C++ 侧扫描根是 :/qt/qml/HMI/Apps（见根目录 HMI_APPS_QML_ROOT），
# 三个 registry 直接扫到 Home / Media / Settings / System 这层目录。
#
# 业务 QML 里的 import HMI.Apps 指向的就是本父模块，
# 各 app 的 QML 用它拿 BasePage / BasePopup / BaseToast。
#
# NO_PLUGIN：静态库形态，模块不生成插件，资源由可执行文件统一承载。
# ============================================================================

qt_add_library(hmi_apps STATIC)

qt_add_qml_module(hmi_apps
    URI "HMI.Apps"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/HMI/Apps"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN

    # --- 大写开头：注册为模块类型，供各 app 模块 import 使用 ---
    QML_FILES
        components/AppButton.qml
        components/BasePage.qml
        components/BasePopup.qml
        components/BaseToast.qml
        components/HeavyTestPage.qml
        components/StatusBar.qml
        components/TitleBar.qml

    # --- 入口（小写，不作为类型）与 JS 辅助：仅打包不注册 ---
    RESOURCES
        main.qml
        components/PopupHelper.js
)

target_link_libraries(hmi_apps PUBLIC Qt6::Quick Qt6::Qml)

# 各业务 app：一个目录 = 一个模块 = 一个 CMakeLists = 一个 owner
add_subdirectory(Home)
add_subdirectory(Media)
add_subdirectory(Settings)
add_subdirectory(System)
