# M15 — C# 脚本（Coral）

## 目标

接入 [StudioCherno/Coral](https://github.com/StudioCherno/Coral)，在 Play 模式下运行 C# `Entity` 脚本（`OnCreate` / `OnUpdate` / `OnDestroy`）。

## 依赖

- **.NET 9 SDK + Runtime**（Coral 当前 `net9.0`）
- `vendor/Coral` 子模块
- CMake 选项 `HZ_SCRIPTING`（默认 ON）

## 构建产物

| 路径 | 内容 |
|------|------|
| `Hazelnut.exe/DotNet/` | `Coral.Managed.dll` + runtimeconfig |
| `Hazelnut.exe/Resources/Scripts/` | `Hazel-ScriptCore.dll` |
| `SampleProject/assets/Scripts/Binaries/` | `Sample.dll`（示例游戏脚本） |

构建时 `HazelDotNetBuild` 目标会自动 `dotnet build` 上述 C# 工程。

## C++ 核心

- `ScriptEngine` — Coral Host、加载 ScriptCore / 项目 DLL、字段元数据、实例化脚本
- `ScriptGlue` — InternalCalls（Transform、Tag、Input、Log、Scene、RigidBody2D）
- `ScriptBuilder` — `dotnet build` 活动项目的 `{Name}.csproj`
- `ScriptStorage` — 每实体脚本字段缓冲，Play 前注入 C# 实例
- `ScriptComponent` — `ScriptID`（FNV 类名哈希）+ 运行时 `CSharpObject`
- `Project::GetScriptModuleFilePath()` — `{ProjectDir}/{ScriptModulePath}/{Name}.dll`

## C# API（`Hazel-ScriptCore`）

- `Entity` — `Tag`、`Translation` / `Rotation` / `Scale`
- `Input.IsKeyPressed` / `IsMouseButtonPressed` / `GetMousePosition`
- `Scene.CreateEntity` / `DestroyEntity` / `FindEntityByTag` / `GetEntities`
- `RigidBody2DComponent` — BodyType、LinearVelocity、GravityScale、ApplyLinearImpulse
- `Log.Info/Warn/Error`
- `[ShowInEditor]` — 私有字段在 Inspector 可见
- 示例脚本：
  - `Sample.Rotator` — 绕 Y 轴旋转（可调 `Speed` 字段）
  - `Sample.Mover` — WASD 平移

## 编辑器用法

1. 打开 `SampleProject`
2. 选中实体 → Add Component → **Script**
3. Script Class 下拉选择 `Sample.Rotator` 或 `Sample.Mover`
4. 调节脚本 public / `[ShowInEditor]` 字段（如 Rotator 的 Speed）
5. **Build → Build C# Scripts**（修改 `.cs` 后）
6. **Build → Reload C# Assembly**（或 Stop Play 后自动 reload；Edit 模式下 DLL 变更会自动热重载）
7. Play 验证行为

## 自测清单

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug --target Hazelnut
ctest --preset=debug
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe
```

- [x] 启动日志：`Coral host initialized`、`Loaded Hazel-ScriptCore`、`Loaded game scripts`
- [x] Hierarchy 中 Script 下拉可见 `Sample.Rotator` / `Sample.Mover`
- [x] Inspector 可编辑 Rotator.Speed 等脚本字段
- [x] Play 后 Rotator 旋转、Mover 响应 WASD
- [x] 场景 YAML 含 `ScriptID` + `ScriptName` + `Fields`
- [x] ctest 含 CSharpScriptTests（Hash / 组件 / 字段序列化）

## 后续扩展（可选）

- Prefab / Asset 类型脚本字段（Material、Texture2D 等）
- C# `GetComponent<T>()` 组件访问 API
- Physics2D Raycast 等高级 API
