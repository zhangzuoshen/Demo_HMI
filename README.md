# Demo_HMI

自研 Qt Quick HMI 框架实验工程：应用容器 / 页面管理 / 生命周期。

## 环境要求

- Qt 6.5+
- CMake 3.16+
- 支持 C++17 的编译器

## 构建

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt6安装路径>/lib/cmake
cmake --build build -j
```

Qt 6 安装路径示例：

- Linux：`~/Qt/6.5.3/gcc_64`
- Windows：`C:/Qt/6.5.3/msvc2019_64`

## 运行

```bash
./build/Demo_HMI
```

嵌入式（EGLFS）下自动使用 OpenGL ES 3.0，桌面使用默认 OpenGL 后端。
