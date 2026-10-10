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

## QML 模块

`apps/` 下的 QML 全部编译进 `HMI.Apps` 模块，资源路径 `qrc:/qt/qml/HMI/Apps/apps/...`：

- 模块内类型互相可见，各文件用 `import HMI.Apps` 导入自己所属的模块
  （QML 文件在 `apps/` 子目录、与模块 qmldir 不同层，隐式导入不生效）
- C++ 侧扫描根路径由 CMake 常量 `HMI_APPS_QML_ROOT` 给出；改 URI 或资源前缀只需动 `CMakeLists.txt`
- 目录式 `import "../../../components"` 已全部移除
- 构建走 qmlcachegen 预编译 QML，并生成 `.qmlls.ini`（Qt Creator 的 QML 语言服务器索引用，已 gitignore）

静态检查：

```bash
~/SDK/Qt6/6.9.3/gcc_64/bin/qmllint -I <构建目录> $(find apps -name "*.qml")
```

`main.qml` 里 `SceneContainer` / `PopupContainer` / `ToastContainer` 会报 "not found"，
属正常：它们是 C++ 运行时注册的类型，qmllint 静态看不到。

## 部署（可选）

```bash
cmake --install build --prefix /tmp/Demo_HMI-install
```

Qt 运行库由 `qt_generate_deploy_app_script` 自动带出。
