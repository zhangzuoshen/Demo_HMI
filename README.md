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

## 部署（可选）

```bash
cmake --install build --prefix /tmp/Demo_HMI-install
```

Qt 运行库由 `qt_generate_deploy_app_script` 自动带出。
