# M23 — LD51 Dichotomy Shell

> 阶段 D 入口：为 LD51 仿制打通 **Parent/Children 层级 + 多实体 Prefab 实例化 + C# 脚本胶水**。

---

## 为何选 LD51

| 候选 | 结论 |
|------|------|
| M14 Jolt | Fragile/Forest 需要，体量大，LD51 不依赖 |
| M20 SoundGraph Phase B | 音频增强，非阶段 D 阻塞 |
| **LD51 Dichotomy** | M17/M18/M21/M22 已就绪；核心缺 **层级 + Prefab 树 + 脚本胶水** |

---

## Phase A（`ebca7e1`）

| 项 | 说明 |
|----|------|
| `HierarchyComponent` | `Parent` UUID + `Children` 列表 |
| YAML 兼容 LD51 | 实体级 `Parent`/`Children`；`TransformComponent.Position` → `Translation` |
| Prefab 根实体 | `Parent == 0` 的实体为根（非首个 PrefabComponent） |
| `Scene::Instantiate` | 递归克隆 Prefab 内**全部实体**，重映射 UUID / BoneEntities / 层级 |
| 测试 | `HierarchyTests` — 反序列化 + 实例化层级 |

---

## Phase B（本提交）

### C++ 引擎

| 项 | 说明 |
|----|------|
| `Scene::SetParent` / `InstantiateChild` | 脚本侧父子挂载 + 子 Prefab 实例化 |
| `AudioCommandRegistry` | 解析 `assets/AudioCommandsRegistry.hzr`（`DebugName` → SoundConfig Handle） |
| `ScriptGlue` 扩展 | Entity 层级、`HasComponent<T>`、Prefab 实例化、`ScriptComponent.Instance`、`Audio.PostEvent` |
| `Project::SetActive` | 加载 AudioCommandsRegistry |

### C# ScriptCore

| 文件 | API |
|------|-----|
| `AssetHandle` / `Asset<T>` / `Prefab` | 编辑器可赋值 Prefab 字段 |
| `Entity` | `Parent`、`Children`、`HasComponent<T>`、`GetComponent<T>`、`Instantiate*`、`As<T>`、`Destroy` |
| `Scene` | `InstantiatePrefab*`、`InstantiatePrefabWithParent*` |
| `Audio` | `AudioCommandID`、`PostEvent`（查 registry → `PlaySoundConfig`） |
| `Mathf` / `Vector3` | LD51 基础数学（Lerp、Distance、XZ/YZ swizzle 等） |

### 测试

- `AudioTest.AudioCommandRegistryLoadsTriggers` — `.hzr` 行解析 + FNV hash 查表

---

## Phase C（后续）

- `DataType::Prefab` 脚本字段序列化
- PNG `LevelReader` + `Hazelnut/LD51Project` 资产子集
- TextComponent 或 ImGui 倒计时 stub
- 完整 Audio API（`PostEventAtLocation`、参数、过滤器等）

---

## LD51 YAML 示例

```yaml
Prefab:
  - Entity: 16876354252294603916
    TagComponent:
      Tag: Player
    Parent: 0
    Children: [12873793010922378982]
    TransformComponent:
      Position: [0, 0, 0]
```

---

## 验证

```powershell
cmake --build --preset=debug --target hazel-engine HazelTests
dotnet build Hazel-ScriptCore/Hazel-ScriptCore.csproj -c Debug
cd build\msvc-debug; ctest -C Debug --output-on-failure
```

---

## 相关模块

- M18 动画 — [module-18-animation.md](./module-18-animation.md)
- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
