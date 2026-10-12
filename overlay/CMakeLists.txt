# ============================================================================
# overlay：弹窗 / Toast 层
#
# 依赖方向：overlay -> core（单向）。
# 用到 core 的 PageManager / Log，但 core 绝不反向依赖 overlay。
# 新增上层模块时保持这个方向，CMake 会自动报出循环依赖。
# ============================================================================

qt_add_library(hmi_overlay STATIC
    NavigationFilter.h  NavigationFilter.cpp
    OverlayContext.h    OverlayContext.cpp
    OverlayPriority.h
    OverlayState.h
    PopupContainer.h    PopupContainer.cpp
    PopupContext.h      PopupContext.cpp
    PopupIncubator.h    PopupIncubator.cpp
    PopupInstance.h
    PopupManager.h      PopupManager.cpp
    PopupRegistry.h     PopupRegistry.cpp
    PopupView.h         PopupView.cpp
    ToastContainer.h    ToastContainer.cpp
    ToastContext.h      ToastContext.cpp
    ToastManager.h      ToastManager.cpp
    ToastRegistry.h     ToastRegistry.cpp
)

target_include_directories(hmi_overlay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hmi_overlay
    PUBLIC
        Qt6::Quick
        hmi_core
)

target_compile_definitions(hmi_overlay PUBLIC QT_DEPRECATED_WARNINGS)
