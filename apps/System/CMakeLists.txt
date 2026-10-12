# ============================================================================
# System：只有系统级弹窗 / Toast 的宿主，manifest entry 为空，不进页面栈
#
# 模块 URI HMI.Apps.System
# 本目录内的 QML 相对路径就是资源路径，
# 改文件名 / 加文件只动这一个文件，不碰根目录 CMakeLists.txt。
# ============================================================================

qt_add_library(hmi_app_system STATIC)

qt_add_qml_module(hmi_app_system
    URI "HMI.Apps.System"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/HMI/Apps/System"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN
    QML_FILES
        popups/HintTip/HintTip.qml
        popups/ThermalWarning/ThermalWarning.qml
        toasts/LowStorage/LowStorage.qml
        toasts/UsbConnected/UsbConnected.qml
    RESOURCES
        manifest.json
        popups/HintTip/manifest.json
        popups/ThermalWarning/manifest.json
        toasts/LowStorage/manifest.json
        toasts/UsbConnected/manifest.json
)

target_link_libraries(hmi_app_system PUBLIC Qt6::Quick Qt6::Qml)
