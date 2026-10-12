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

# Qt 6.3+：自动开启 AUTOMOC / AUTOUIC
qt_standard_project_setup()

# ---------------------------------------------------------------
# QTP0004：Qt 6.9 要求"模块内每个含 .qml 的子目录都有自己的 qmldir"。
#
# 现状：拆成多模块后，Home / Settings / components 已是"一个模块一个目录"，
# 但 Media 与 System 仍把弹窗、Toast 放在 popups/<id>/、toasts/<id>/ 子目录里
# （三个 registry 靠这套目录结构扫描），给每个弹窗再开一个模块过于琐碎，
# 所以保留旧行为：子目录里的 QML 由模块根 qmldir 的带路径条目声明。
#
# 注意：Qt 提示 OLD 行为会在未来大版本移除。届时的两个出路：
#   a) 给每个 popups/<id>/、toasts/<id>/ 补 qmldir（拆成子模块）；
#   b) 把弹窗 / Toast 拍平到模块根下，registry 改按 manifest 定位。
# ---------------------------------------------------------------
qt_policy(SET QTP0004 OLD)

# 走 QML 模块后这个开关才真正生效：构建时生成 qmlls.ini，
# Qt Creator 的 QML 语言服务器才能正确索引本工程
set(QT_QML_GENERATE_QMLLS_INI ON)

# ---------------------------------------------------------------
# 所有 QML 模块共用 URI 根 HMI.Apps，资源落在
#   qrc:/qt/qml/HMI/Apps/...
# C++ 侧三个 registry 的扫描根即 :/qt/qml/HMI/Apps，
# 直接扫到 Home / Media / Settings / System 这层目录。
# 改 URI 或资源前缀只动这一处。
# ---------------------------------------------------------------
set(HMI_APPS_URI "HMI.Apps")
string(REPLACE "." "/" HMI_APPS_URI_PATH "${HMI_APPS_URI}")
set(HMI_APPS_QML_ROOT ":/qt/qml/${HMI_APPS_URI_PATH}")

# ---------------------------------------------------------------
# 子模块：一个目录 = 一个 CMakeLists = 一个 owner
#   core/    框架核心（页面 / App 生命周期）
#   overlay/ 弹窗 / Toast
#   apps/    QML 模块树（父模块 + 各业务 app）
#
# 平时加业务代码只动对应子目录的 CMakeLists，根目录这个文件几乎不碰，
# 从根上避免多人改同一份列表造成的冲突。
# ---------------------------------------------------------------
add_subdirectory(core)
add_subdirectory(overlay)
add_subdirectory(apps)

# 可执行文件：只放入口与装配（bootstrap 同时依赖 core 与 overlay，
# 放哪边都会造成反向依赖，所以留在这一层）
qt_add_executable(Demo_HMI
    main.cpp
    ApplicationBootstrap.h
    ApplicationBootstrap.cpp
)

target_include_directories(Demo_HMI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Demo_HMI
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        # C++ 库：符号被可执行文件真实引用，常规链接即可
        hmi_core
        hmi_overlay
        # QML 模块：静态库里未被引用符号的 object 会被链接器丢弃，
        # 而资源的自动注册就写在那些 object 里，所以必须 whole-archive
        # 强制全部链入，否则运行时报 "module ... is not installed"。
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,hmi_apps>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,hmi_app_home>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,hmi_app_media>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,hmi_app_settings>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,hmi_app_system>"
)

target_compile_definitions(Demo_HMI PRIVATE
    QT_DEPRECATED_WARNINGS
    # C++ 侧扫描 apps 的根路径，唯一真相源（随 URI / 前缀变化）
    HMI_APPS_QML_ROOT="${HMI_APPS_QML_ROOT}"
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
