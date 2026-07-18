# Hazel 项目 — Agent 指南

本文档供 AI Agent 与协作者快速理解仓库结构、构建方式与代码约定。项目基于 [Cherno Hazel 系列](https://www.youtube.com/playlist?list=PLlrATfBNZ98dTKPaX9Y3HvC4qRyAm8Hbj) 教程，使用 **CMake + vcpkg manifest** 替代原 Premake 工作流。

---

## 1. 仓库总览

| 组件 | 路径 | CMake 目标 | 说明 |
|------|------|------------|------|
| **引擎库** | `Hazel/` | `hazel-engine`（别名 `Hazel::Engine`） | 静态库，核心运行时 |
| **沙盒应用** | `Sandbox/` | `Sandbox` | 链接引擎的示例/测试 exe |
| **编辑器（预留）** | `Hazelnut/` | *尚未接入 CMake* | 空目录，对应教程后期 Editor 项目 |
| **构建脚本** | `cmake/` | — | 共享 CMake 模块 |
| **依赖** | `vendor/vcpkg` + `vcpkg.json` | — | vcpkg 子模块 + manifest |

根目录 `CMakeLists.txt` 通过 `HAZEL_BUILD_SANDBOX`（默认 ON）控制是否构建 Sandbox。

---

## 2. 目录结构

```
Hazel/                          # 仓库根（project 名也为 Hazel）
├── AGENTS.md                   # 本文件
├── README.md                   # 人类可读：依赖、首次构建
├── CMakeLists.txt              # 根 CMake：引擎 + Sandbox
├── CMakePresets.json           # windows-msvc-debug / release 预设
├── vcpkg.json                  # manifest 依赖声明
├── cmake/
│   └── HazelOutputDirs.cmake   # bin / bin-int 输出目录（对齐 Premake）
├── Hazel/                      # 引擎源码
│   ├── CMakeLists.txt
│   └── src/
│       ├── Hazel.h             # 应用入口头（Application + EntryPoint）
│       ├── hzpch.h / hzpch.cpp # 预编译头
│       ├── Hazel/              # 引擎逻辑（见 §3）
│       └── Platform/           # 平台实现（见 §4）
├── Sandbox/                    # 客户端应用
│   ├── CMakeLists.txt
│   ├── src/SandboxApp.cpp      # CreateApplication() 实现
│   └── assets/                 # 运行时资源（构建时复制到 exe 旁）
├── Hazelnut/                   # 预留：未来 Editor，当前为空
├── vendor/
│   ├── vcpkg/                  # git submodule
│   ├── imgui/                  # git submodule（ocornut/imgui，docking 分支）
│   ├── ImGuizmo/               # git submodule
│   └── vcpkg_installed/        # 自动生成，勿提交
├── build/                      # CMake 生成文件（勿提交）
├── bin/                          # 最终产物 exe / lib
│   └── <Config>-windows-x86_64/
│       ├── Hazel/hazel-engine.lib
│       └── Sandbox/Sandbox.exe + assets/
└── bin-int/                      # 中间 .obj（部分重定向至此）
```

### 构建产物路径

- 可执行文件 / 库：`bin/<Config>-windows-x86_64/<ProjectName>/`
- 中间对象文件：`bin-int/<Config>-windows-x86_64/<ProjectName>/`（通过 `/Fo`）
- CMake 缓存与 VS 工程：`build/msvc-debug/` 或 `build/msvc-release/`

---

## 3. 引擎源码 `Hazel/src/Hazel/`

| 模块 | 路径 | 职责 |
|------|------|------|
| **Core** | `Core/` | `Application`、`Layer`/`LayerStack`、`Window`、`Log`、`Input`、`Timestep`、宏与 `Ref`/`Scope` |
| **Events** | `Events/` | `Event`、`EventDispatcher`、键盘/鼠标/窗口事件 |
| **Renderer** | `Renderer/` | 抽象渲染 API、`Renderer2D`、Camera、Shader、Buffer、Texture、Framebuffer |
| **Scene** | `Scene/` | EnTT ECS：`Scene`、`Entity`、组件、`SceneSerializer`（yaml-cpp） |
| **ImGui** | `ImGui/` | `ImGuiLayer`（Dear ImGui + GLFW + OpenGL3） |
| **Debug** | `Debug/` | `Instrumentor.h` 性能分析（Chrome trace JSON） |
| **Utils** | `Utils/` | `PlatformUtils.h`（Windows 文件对话框等） |

### 应用生命周期

1. `EntryPoint.h` 中 `main` → `Log::Init()` → `CreateApplication()`（**在客户端定义**，如 `SandboxApp.cpp`）
2. `Application` 构造：创建 `Window`、`Renderer::Init()`、挂载 `ImGuiLayer`
3. `Run()` 循环：`LayerStack::OnUpdate` → ImGui Begin/Render/End → `Window::OnUpdate`（swap buffers）
4. 事件经 `OnEvent` 从顶层 Layer 向下分发

### 关键类型约定

- `Ref<T>` = `std::shared_ptr<T>`，`Scope<T>` = `std::unique_ptr<T>`（定义于 `Core/Core.h`）
- 渲染资源工厂：`VertexBuffer::Create(...)`、`Shader::Create(...)` 等，内部按 `RendererAPI` 分发到 OpenGL 实现
- 基类解绑方法命名为 **`UnBind`**（大写 B），与部分 OpenGL 实现保持一致
- EnTT 头文件：`#include <entt/entt.hpp>`（vcpkg 路径，不是 `entt.hpp`）
- 日志宏：`HZ_CORE_*` / `HZ_*`；严重级别用 spdlog 的 **`critical()`**（不是 `fatal()`）

---

## 4. 平台层 `Hazel/src/Platform/`

| 路径 | 说明 |
|------|------|
| `Windows/WindowsWindow.cpp` | GLFW 窗口、`GraphicsContext` 创建、回调 |
| `Windows/WindowsInput.cpp` | `Input` 平台实现 |
| `Windows/WindowsPlatformUtils.cpp` | 原生文件对话框 |
| `OpenGL/OpenGL*.cpp` | OpenGL 版 Buffer、Shader、Texture、VertexArray、Framebuffer、RendererAPI、Context |

**平台限制：** 当前仅 `HZ_PLATFORM_WINDOWS` + OpenGL。`Core.h` 在非 Windows 下 `#error`。

**OpenGL 头文件顺序：** 必须先 `#include <glad/glad.h>`，再 `#include <GLFW/glfw3.h>`，否则 glad 报 “OpenGL header already included”。

---

## 5. Sandbox 客户端

### 文件

- `src/SandboxApp.cpp`：继承 `Application`，实现 `Hazel::CreateApplication()`
- `assets/`：相对路径资源，构建后复制到 `bin/.../Sandbox/assets/`

### 资源路径约定

运行时以 **exe 所在目录为工作目录**，使用相对路径，例如：

- `assets/shaders/Texture.glsl` — `Renderer2D::Init()` 加载
- `assets/fonts/opensans/OpenSans-*.ttf` — `ImGuiLayer` 加载

`Sandbox/CMakeLists.txt` 中 `POST_BUILD` 会将 `assets/` 复制到输出目录。**新增资源后需重新 build Sandbox**。

### 当前状态

默认 `SandboxApp` 为空壳：无自定义 Layer、无 `Renderer2D` 绘制、ImGui Demo 未启用 → **窗口黑屏属正常**。鼠标移动会在日志中产生 `MouseMovedEvent`。

### 性能日志

`EntryPoint.h` 将 profile JSON 写到 `./log/`（相对 cwd），默认即 `Sandbox/log/`（若从 exe 目录运行则需在彼处建 `log` 目录）。

---

## 6. Hazelnut（编辑器）

- 路径：`Hazelnut/`
- **当前为空目录**，未加入根 `CMakeLists.txt`
- 教程中后期独立 Editor 项目；接入时可参照 Sandbox：`add_subdirectory(Hazelnut)` + 链接 `Hazel::Engine`
- Agent 不应假设 Hazelnut 内已有代码

---

## 7. 依赖（vcpkg.json）

| 包 | 用途 |
|----|------|
| spdlog | 日志 |
| glfw3 | 窗口与输入 |
| glad | OpenGL 加载 |
| entt | ECS |
| glm | 数学 |
| imgui / ImGuizmo | **`vendor/imgui`**（docking 分支）+ **`vendor/ImGuizmo`**；Docking + 多视口 |
| stb | 图像加载（需在**一个** `.cpp` 中 `#define STB_IMAGE_IMPLEMENTATION`） |
| yaml-cpp | 场景序列化 |

ImGui 来自 **`vendor/imgui`**（[ocornut/imgui](https://github.com/ocornut/imgui) `docking` 分支，CMake 见 `cmake/HazelImGui.cmake`）；`ImGuiLayer` 启用 Docking/Viewports，`EditorLayer` 提供 `DockSpace`。

---

## 8. 构建命令

```powershell
# 首次
git submodule update --init --recursive
cd vendor/vcpkg && .\bootstrap-vcpkg.bat && cd ..\..

# 配置 + 构建
cmake --preset=windows-msvc-debug
cmake --build --preset=debug

# 运行
.\bin\Debug-windows-x86_64\Sandbox\Sandbox.exe
```

可选 CMake 变量：`HAZEL_BUILD_SANDBOX=OFF` 仅构建引擎。

---

## 9. Agent 修改代码时的注意事项

### 应做

- 新 `.cpp/.h` 放在 `Hazel/src/` 下会被 `GLOB_RECURSE` 自动纳入引擎，**无需改 CMake**（Sandbox 的 cpp 需手动加入 `SANDBOX_SOURCES`）
- 使用引擎公开头：`#include <Hazel.h>` 或 `#include "Hazel/..."`（include 根为 `Hazel/src`）
- 需要 `Ref`、`Assert` 时包含 `Hazel/Core/Core.h`
- 修改 Sandbox 资源后触发 Sandbox 目标 rebuild 以复制 assets
- 保持 C++17，与现有命名空间 `Hazel` 一致

### 应避免

- 在 `vendor/vcpkg_installed/` 或 `build/` 中改文件
- 将 `COMPILE_PDB_OUTPUT_DIRECTORY` 重定向到共享目录而不加 `/FS`（会触发 MSVC C1041；当前 `HazelOutputDirs.cmake` 已不对编译 PDB 重定向）
- 并行构建时若 PCH 被占用：结束残留 `cl.exe` 后重试
- 在头文件中使用 `entt.hpp`、spdlog `fatal()`、OpenGL 与 glad 错误包含顺序

### MSVC 并行编译

若遇 `hazel-engine.pdb` C1041：检查 `cmake/HazelOutputDirs.cmake` 是否误设 `COMPILE_PDB_OUTPUT_DIRECTORY`；obj 可放在 `bin-int`，编译 PDB 应留在 `build/.../hazel-engine.dir/`。

---

## 10. 架构简图

```
Sandbox.exe
    └── links Hazel::Engine (static)
            ├── Core (Application, Layer, Window, Log, Input)
            ├── Events
            ├── Renderer ──► RendererAPI ──► OpenGLRendererAPI
            ├── Scene (EnTT ECS, SceneSerializer)
            ├── ImGuiLayer
            └── Platform (WindowsWindow, WindowsInput, OpenGL*)
```

---

## 11. 相关文档

| 文件 | 内容 |
|------|------|
| `README.md` | 依赖安装、构建、运行（面向开发者） |
| `Hazel/AGENTS.md` | 见根目录本文 §3–§4 |
| `Sandbox/AGENTS.md` | 见根目录本文 §5 |

---

## 12. 常见任务速查

| 任务 | 位置 / 做法 |
|------|-------------|
| 添加游戏 Layer | Sandbox 新建类继承 `Layer`，`PushLayer` / `PushOverlay` |
| 2D 绘制 | `Renderer2D::BeginScene` / `DrawQuad` / `EndScene` |
| 新 Shader | `assets/shaders/*.glsl`，`Shader::Create("assets/shaders/...")` |
| 新第三方库 | 编辑 `vcpkg.json` + `Hazel/CMakeLists.txt` `find_package` / `target_link_libraries` |
| 接入 Hazelnut | 创建 `Hazelnut/CMakeLists.txt`，根 CMake `add_subdirectory`，参照 Sandbox |
