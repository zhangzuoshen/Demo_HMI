# ============================================================================
# Settings 应用
#
# 模块 URI HMI.Apps.Settings
# 本目录内的 QML 相对路径就是资源路径，
# 改文件名 / 加文件只动这一个文件，不碰根目录 CMakeLists.txt。
# ============================================================================

qt_add_library(hmi_app_settings STATIC)

qt_add_qml_module(hmi_app_settings
    URI "HMI.Apps.Settings"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/HMI/Apps/Settings"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN
    QML_FILES
        SettingsApp.qml
    RESOURCES
        manifest.json
)

target_link_libraries(hmi_app_settings PUBLIC Qt6::Quick Qt6::Qml)
