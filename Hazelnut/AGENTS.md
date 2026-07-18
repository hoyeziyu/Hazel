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
- Viewport **ImGuizmo Gizmo**（Q/W/E/R 切换 无/平移/旋转/缩放；Ctrl 吸附）
- 示例场景：`assets/scenes/Example.hazel`

## 说明

- 使用 vcpkg **标准 imgui**（无 Docking），UI 为 MainMenuBar + 独立窗口布局
- 详见根目录 [`../AGENTS.md`](../AGENTS.md) §6
