# M23 — LD51 Dichotomy Shell（Phase A：实体层级）

> 阶段 D 入口：为 LD51 仿制打通 **Parent/Children 层级 + 多实体 Prefab 实例化**，后续 Phase B 再接 C# Prefab API 与 PNG 关卡。

---

## 为何选 LD51

| 候选 | 结论 |
|------|------|
| M14 Jolt | Fragile/Forest 需要，体量大，LD51 不依赖 |
| M20 SoundGraph Phase B | 音频增强，非阶段 D 阻塞 |
| **LD51 Dichotomy** | M17/M18/M21/M22 已就绪；核心缺 **层级 + Prefab 树 + 脚本胶水** |

---

## Phase A（本提交）

| 项 | 说明 |
|----|------|
| `HierarchyComponent` | `Parent` UUID + `Children` 列表 |
| YAML 兼容 LD51 | 实体级 `Parent`/`Children`；`TransformComponent.Position` → `Translation` |
| Prefab 根实体 | `Parent == 0` 的实体为根（非首个 PrefabComponent） |
| `Scene::Instantiate` | 递归克隆 Prefab 内**全部实体**，重映射 UUID / BoneEntities / 层级 |
| 测试 | `HierarchyTests` — 反序列化 + 实例化层级 |

---

## Phase B（后续）

- C# `Entity.Children` / `Prefab.Instantiate` / `GetComponent<T>`
- `AudioCommandsRegistry` + `Audio.PostEvent`
- PNG `LevelReader` + `LD51Project` 资产子集
- TextComponent 或 ImGui 倒计时 stub

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
cmake --build --preset=debug --target HazelTests
cd build\msvc-debug; ctest -C Debug -R HierarchyTest --output-on-failure
```

---

## 相关模块

- M18 动画 — [module-18-animation.md](./module-18-animation.md)
- M22 Play-from-Pack — [module-22-play-from-pack.md](./module-22-play-from-pack.md)
