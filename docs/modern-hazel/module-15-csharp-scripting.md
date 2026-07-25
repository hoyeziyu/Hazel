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

- `ScriptEngine` — Coral Host、加载 ScriptCore / 项目 DLL、实例化脚本
- `ScriptGlue` — InternalCalls（Transform、Scene 校验、Log）
- `ScriptComponent` — `ScriptID`（FNV 类名哈希）+ 运行时 `CSharpObject`
- `Project::GetScriptModuleFilePath()` — `{ProjectDir}/{ScriptModulePath}/{Name}.dll`

## C# API（精简版 `Hazel-ScriptCore`）

- `Entity` — `Translation` / `Rotation`、`OnCreate` / `OnUpdate` / `OnDestroy`
- `Log.Info/Warn/Error`
- 示例：`Sample.Rotator` — Play 时绕 Y 轴旋转

## 编辑器用法

1. 打开 `SampleProject`
2. 选中实体 → Add Component → **Script**
3. Script Class 下拉选择 `Sample.Rotator`
4. Play — 控制台应出现 `Rotator attached to entity ...`

## 自测清单

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug --target Hazelnut
ctest --preset=debug
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe
```

- [ ] 启动日志：`Coral host initialized`、`Loaded Hazel-ScriptCore`、`Loaded game scripts`
- [ ] Hierarchy 中 Script 下拉可见 `Sample.Rotator`
- [ ] Play 后实体旋转，Stop 后脚本销毁无崩溃

## 后续扩展

- ScriptStorage / 序列化脚本字段
- 更多 InternalCalls（Input、Physics2D、Scene API）
- ScriptBuilder 热重载、`Reload C# Assembly` 菜单
