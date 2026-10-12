# ============================================================================
# Home 应用
#
# 模块 URI HMI.Apps.Home
# 本目录内的 QML 相对路径就是资源路径，
# 改文件名 / 加文件只动这一个文件，不碰根目录 CMakeLists.txt。
# ============================================================================

qt_add_library(hmi_app_home STATIC)

qt_add_qml_module(hmi_app_home
    URI "HMI.Apps.Home"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/HMI/Apps/Home"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN
    QML_FILES
        HomeApp.qml
    RESOURCES
        manifest.json
)

target_link_libraries(hmi_app_home PUBLIC Qt6::Quick Qt6::Qml)
