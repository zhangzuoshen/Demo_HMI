# Demo_HMI

自研 Qt Quick HMI 框架实验工程：应用容器 / 页面管理 / 生命周期。

## 环境要求

- Qt 6.9+（当前开发机 SDK：`~/SDK/Qt6/6.9.3/gcc_64`）
- CMake 3.22+（Qt 6.9 的 `QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_USING_QT`；低于 3.21 会被 `Qt6Config.cmake` 直接拒绝）
- 支持 C++17 的编译器（Qt 6.9 的 ABI 基线仍是 C++17）

## 构建

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/SDK/Qt6/6.9.3/gcc_64
cmake --build build -j$(nproc)
```

Qt 安装路径示例：

- Linux / WSL：`~/SDK/Qt6/6.9.3/gcc_64`
- Windows：`C:/Qt/6.9.3/msvc2022_64`

系统 cmake 版本不够时，用 Qt 自带的：`~/SDK/Qt6/Tools/CMake/bin/cmake`（3.30.5）。

## 运行

```bash
./build/Demo_HMI
```

无显示器环境（WSL / CI）用 offscreen 平台：

```bash
QT_QPA_PLATFORM=offscreen ./build/Demo_HMI
```

嵌入式（EGLFS）下自动使用 OpenGL ES 3.0，桌面使用默认 OpenGL 后端。

## 工程结构

```
Demo_HMI/
├── CMakeLists.txt          # 只有全局配置 + add_subdirectory + 可执行文件
├── main.cpp
├── ApplicationBootstrap.*  # 装配层（同时依赖 core 与 overlay，故留在这一层）
├── core/                   # 框架核心：页面 / App 生命周期      -> hmi_core
├── overlay/                # 弹窗 / Toast                       -> hmi_overlay
└── apps/                   # QML 模块树
    ├── CMakeLists.txt      # 父模块 HMI.Apps（入口 main.qml + 共享组件）
    ├── components/         #   BasePage / BasePopup / BaseToast / ...
    ├── Home/               # 模块 HMI.Apps.Home
    ├── Media/              # 模块 HMI.Apps.Media（含自己的 popups/ toasts/）
    ├── Settings/           # 模块 HMI.Apps.Settings
    └── System/             # 模块 HMI.Apps.System（只有弹窗 / Toast）
```

**一个目录 = 一个 CMakeLists = 一个 owner。** 平时加业务代码只动对应子目录的
`CMakeLists.txt`，根目录那个几乎不碰 —— 从根上避免多人改同一份长列表导致的冲突。

依赖方向：`overlay -> core`，`core` 是依赖图叶子、不反向依赖任何上层。
方向由 `target_link_libraries(... PUBLIC ...)` 声明，一旦写反 CMake 会直接报循环依赖。

## QML 模块

| 模块 | 目录 | 内容 | owner |
|---|---|---|---|
| `HMI.Apps` | `apps/` | 入口 `main.qml` + 共享组件 | 架构 |
| `HMI.Apps.Home` | `apps/Home/` | Home 应用 | Home |
| `HMI.Apps.Media` | `apps/Media/` | Media 应用 + 自己的 popups/ toasts/ | Media |
| `HMI.Apps.Settings` | `apps/Settings/` | Settings 应用 | Settings |
| `HMI.Apps.System` | `apps/System/` | 系统级弹窗 / Toast（entry 为空，不进页面栈） | 架构 |

- 资源路径 `qrc:/qt/qml/HMI/Apps/...`
- C++ 三个 registry 的扫描根是 `:/qt/qml/HMI/Apps`（CMake 常量 `HMI_APPS_QML_ROOT`），
  直接扫到 `Home/ Media/ Settings/ System` 这层目录。改 URI 或资源前缀只动根目录 `CMakeLists.txt`
- 业务 QML 用 `import HMI.Apps` 拿共享组件（`BasePage` / `BasePopup` / `BaseToast`）；
  C++ 类型在 `HMI.Core`，由 `qmlRegisterType` 运行时注册
- 新增 app：建目录 → 写自己的 `CMakeLists.txt` → 在 `apps/CMakeLists.txt` 加一行
  `add_subdirectory` → 在根 `CMakeLists.txt` 的链接列表加一条 WHOLE_ARCHIVE
- 构建走 qmlcachegen 预编译 QML，并生成 `.qmlls.ini`
  （Qt Creator 的 QML 语言服务器索引用，已 gitignore）

> **静态库形态的 QML 模块必须 whole-archive 链接。**
> 模块资源的自动注册写在静态库生成的 object 里，而链接器会丢弃"没有被引用符号"的
> object —— 那些 object 不会被链进可执行文件，运行时报
> `module ... is not installed`，且编译期毫无征兆。
> 所以可执行文件里这些模块都用 `$<LINK_LIBRARY:WHOLE_ARCHIVE,...>` 强制全部链入。
> **新增 app 模块时务必同步加这一条。**

静态检查：

```bash
~/SDK/Qt6/6.9.3/gcc_64/bin/qmllint -I <构建目录>/qml $(find apps -name "*.qml")
```

`main.qml` 里 `SceneContainer` / `PopupContainer` / `ToastContainer` 会报 "not found"，
属正常：它们是 C++ 运行时注册的类型（`HMI.Core`），qmllint 静态看不到。

## 部署（可选）

```bash
cmake --install build --prefix /tmp/Demo_HMI-install
```

Qt 运行库由 `qt_generate_deploy_app_script` 自动带出。
