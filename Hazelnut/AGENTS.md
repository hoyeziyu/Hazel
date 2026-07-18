# Hazelnut（编辑器）

Cherno 教程 **Hazel 编辑器**，链接 `Hazel::Engine`。

## 构建与运行

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug --target Hazelnut
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe
```

- CMake 开关：`HAZEL_BUILD_HAZELNUT`（根 `CMakeLists.txt`，默认 ON）
- 产物：`bin/<Config>-windows-x86_64/Hazelnut/Hazelnut.exe`
- 资源：`Hazelnut/assets/` 构建时 POST_BUILD 复制到 exe 旁

## 功能

- Scene Viewport（Framebuffer 离屏渲染）
- Scene Hierarchy + Properties 面板
- File → New / Open / Save As（`.hazel` 场景序列化）
- **项目系统（M1）**：`.hzproj` — New / Open / Save / Close Project；启动加载 `SampleProject/`
- **Play 模式（M2）**：Edit/Play 双场景；Viewport Play/Stop + `Alt+P`
- **Content Browser（M3）**：浏览 `assets/`；双击 `.hazel` 打开场景
- **Editor Camera（M4）**：Edit 用透视 EditorCamera；Play 用场景 Primary Camera
- Viewport **ImGuizmo Gizmo**（Q/W/E/R，Edit 模式，透视矩阵）
- 示例场景：`assets/scenes/Example.hazel`

## 说明

- ImGui 来自 **`vendor/imgui`**（docking）+ **`vendor/ImGuizmo`**；`DockSpace Demo` + 可停靠面板
- 详见根目录 [`../AGENTS.md`](../AGENTS.md) §6
