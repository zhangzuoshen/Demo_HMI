# ============================================================================
# Media 应用（含自己的弹窗与 Toast）
#
# 模块 URI HMI.Apps.Media
# 本目录内的 QML 相对路径就是资源路径，
# 改文件名 / 加文件只动这一个文件，不碰根目录 CMakeLists.txt。
# ============================================================================

qt_add_library(hmi_app_media STATIC)

qt_add_qml_module(hmi_app_media
    URI "HMI.Apps.Media"
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/HMI/Apps/Media"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    NO_PLUGIN
    QML_FILES
        MediaApp.qml
        popups/TrackInfo/TrackInfo.qml
        toasts/PlayFailed/PlayFailed.qml
        toasts/VolumeChanged/VolumeChanged.qml
    RESOURCES
        manifest.json
        popups/TrackInfo/manifest.json
        toasts/PlayFailed/manifest.json
        toasts/VolumeChanged/manifest.json
)

target_link_libraries(hmi_app_media PUBLIC Qt6::Quick Qt6::Qml)
