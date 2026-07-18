# Hazel 引擎 — Project 模块

M1 缩小版项目系统，对照 StudioCherno `Hazel/Project/`。

## 类型

- `ProjectConfig` — Name、AssetDirectory、StartScene、ProjectDirectory
- `Project` — 当前活动项目（`GetActive()` / `SetActive()`）
- `ProjectSerializer` — `.hzproj` YAML 读写

## 路径

```cpp
project->GetAssetDirectory();           // {ProjectDir}/assets
project->GetStartScenePath();           // {ProjectDir}/assets/scenes/Main.hazel
Project::GetRuntimeDirectory();         // exe 目录
```

## 编辑器

见 `Hazelnut/AGENTS.md` 与 `docs/modern-hazel/module-01-project.md`。
