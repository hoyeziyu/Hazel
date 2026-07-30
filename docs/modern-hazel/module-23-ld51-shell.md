# M23 — LD51 Dichotomy Shell

> 阶段 D 入口：为 LD51 仿制打通 **Parent/Children 层级 + 多实体 Prefab 实例化 + C# 脚本胶水 + PNG 关卡读取**。

---

## Phase A（`ebca7e1`）

| 项 | 说明 |
|----|------|
| `HierarchyComponent` | `Parent` UUID + `Children` 列表 |
| YAML 兼容 LD51 | 实体级 `Parent`/`Children`；`TransformComponent.Position` → `Translation` |
| Prefab 根实体 | `Parent == 0` 的实体为根 |
| `Scene::Instantiate` | 递归克隆 Prefab 内全部实体 |
| 测试 | `HierarchyTests` |

---

## Phase B（`0b15da0`）

| 项 | 说明 |
|----|------|
| `AudioCommandRegistry` | `.hzr` 解析 + `Audio.PostEvent` |
| C# ScriptCore | `Prefab`、`Entity.Children/GetComponent`、`Scene.InstantiatePrefab*` |
| `ScriptGlue` | 层级、HasComponent、Prefab 实例化、Audio |

---

## Phase C（本提交）

| 项 | 说明 |
|----|------|
| `DataType::Prefab` | 脚本字段 YAML 序列化；Inspector 编辑 AssetHandle |
| `AudioCommandsRegistry.hzr` | SampleProject + LD51Project 预置 |
| `Hazelnut/LD51Project` | LD51 资产子集：`LD51.hzproj`、场景壳、`LevelReader.cs`、示例 `Levels/Starter.png` |
| LD51 ACR 格式 | 兼容扁平 `Target:` 与嵌套 `Actions:` 块 |

### 打开 LD51 项目

```powershell
.\bin\Debug-windows-x86_64\Hazelnut\Hazelnut.exe Hazelnut\LD51Project\LD51.hzproj
```

### 构建 LD51 脚本

```powershell
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
```

---

## Phase D（后续）

- 从 LD51 主仓库导入 Prefab / 材质 / 动画资产
- `LevelManager` 脚本 + Prefab 映射表
- TextComponent 或 ImGui 倒计时 stub

---

## 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests Hazelnut
dotnet build Hazel-ScriptCore/Hazel-ScriptCore.csproj -c Debug
dotnet build Hazelnut/LD51Project/assets/Scripts/LD51.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
```

---

## 相关模块

- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
