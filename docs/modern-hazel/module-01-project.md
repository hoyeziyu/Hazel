# M1 — 项目系统

> 对照：`HazelEngine/Hazel/src/Hazel/Project/`、`Sandbox.hproj`  
> 实现：`Hazel/src/Hazel/Project/`、`Hazelnut/SampleProject/`

## StudioCherno 怎么做

- 项目描述文件：`.hproj`（YAML），例如 `Sandbox.hproj`
- 核心字段：`Name`、`AssetDirectory`、`StartScene`、物理/脚本/日志等大量配置
- `Project::SetActive()` 设置当前项目；路径相对 **项目根目录**
- `EditorLayer::OpenProject()` → 反序列化 → 加载 `StartScene` → 通知各 Panel
- 新建项目：复制 `NewProjectTemplate`，生成目录树 + C# 工程（我们暂不实现）

参考文件：

```
HazelEngine/Hazel/src/Hazel/Project/Project.h
HazelEngine/Hazel/src/Hazel/Project/ProjectSerializer.cpp
HazelEngine/Hazelnut/SandboxProject/Sandbox.hproj
HazelEngine/Hazelnut/src/EditorLayer.cpp  → OpenProject / CreateProject
```

## 我们的缩小版实现

### 新增引擎模块

| 文件 | 职责 |
|------|------|
| `Hazel/Project/Project.h` | `ProjectConfig`、`Project::GetActive()`、路径解析 |
| `Hazel/Project/ProjectSerializer.*` | `.hzproj` YAML 读写 |
| `Project::GetRuntimeDirectory()` | exe 所在目录（用于启动 SampleProject） |

### 项目文件格式（`.hzproj`）

扩展名用 `.hzproj` 与 StudioCherno 的 `.hproj` 区分，结构刻意保持相似：

```yaml
Project:
  Name: Sample
  AssetDirectory: assets
  StartScene: scenes/Main.hazel
```

路径约定：

- `ProjectDirectory` = `.hzproj` 文件所在目录
- `StartScene` = 相对 `AssetDirectory` 的路径
- 完整场景路径 = `{ProjectDirectory}/{AssetDirectory}/{StartScene}`

### 编辑器改动（`Hazelnut/src/EditorLayer.cpp`）

| 菜单项 | 行为 |
|--------|------|
| New Project... | 选路径创建 `.hzproj` + `assets/scenes/Main.hazel` |
| Open Project... | `Ctrl+Shift+O`，加载项目并打开 StartScene |
| Save Project | 写回 `.hzproj`（当前打开场景会更新为 StartScene） |
| Close Project | 保存并清除 `Project::GetActive()` |

启动时自动尝试打开：

```
{exe目录}/SampleProject/Sample.hzproj
```

### 示例项目

```
Hazelnut/SampleProject/
├── Sample.hzproj
└── assets/scenes/Main.hazel   # 绿/红方块 + 相机
```

构建时 POST_BUILD 复制到 `bin/.../Hazelnut/SampleProject/`。

## 与 StudioCherno 的差异（M1 刻意未做）

| 功能 | StudioCherno | 我们 |
|------|-------------|------|
| 扩展名 | `.hproj` | `.hzproj` |
| C# / Coral | ScriptModulePath、自动编译 | 无 |
| AssetRegistry | `.hzr` 注册表 | 无 |
| 物理/音频/日志配置 | Project YAML 内 | 无 |
| Recent Projects | UserPreferences | 无 |
| Content Browser 联动 | `OnProjectChanged` | 无（M3） |

## 验收清单

- [ ] 启动 Hazelnut，自动加载 SampleProject，Viewport 有绿/红方块
- [ ] **Project** 面板显示项目名、根目录、StartScene
- [ ] File → Open Project，选择其他 `.hzproj` 能切换
- [ ] File → New Project，创建目录 + 默认 Main 场景
- [ ] File → Save Project，修改 StartScene 后写回 YAML
- [ ] File → Close Project，Project 面板消失，场景清空

## 构建与运行

```powershell
cmake --build --preset=debug --target Hazelnut
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe
```

## 下一模块（M2）

Play 模式：对照 `EditorLayer` 中 `SceneState::Edit/Play` 与 `m_EditorScene` / `m_RuntimeScene` 分离。
