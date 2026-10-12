# ============================================================================
# core：框架核心层（页面 / App 生命周期）
#
# 依赖方向：core 不依赖任何上层（无 overlay / 无 QML），
# 是依赖图的叶子节点，可被任意上层引用而不产生环。
# ============================================================================

qt_add_library(hmi_core STATIC
    AppContext.h        AppContext.cpp
    AppInstance.h       AppInstance.cpp
    AppRegistry.h       AppRegistry.cpp
    AppState.h
    Log.h               Log.cpp
    PageIncubator.h     PageIncubator.cpp
    PageManager.h       PageManager.cpp
    PageView.h          PageView.cpp
    SceneContainer.h    SceneContainer.cpp
    WindowState.h
)

# PUBLIC：core 的头文件对上层可见（overlay 要用 PageManager.h / Log.h）
target_include_directories(hmi_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hmi_core PUBLIC Qt6::Quick)

target_compile_definitions(hmi_core PUBLIC QT_DEPRECATED_WARNINGS)
