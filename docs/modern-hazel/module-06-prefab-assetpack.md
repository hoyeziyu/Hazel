# M6 — Prefab 与 AssetPack

> 对照：`HazelEngine/Hazel/src/Hazel/Scene/Prefab.*`、`Serialization/AssetPack*`  
> 实现：Prefab 资产、`.hprefab`、`.hap` 运行时包、Build 菜单

## 目标

在 M5 Handle 体系上扩展：

1. **Prefab**：可复用实体模板（`.hprefab`）
2. **AssetPack**：把 Scene + 依赖资产烘焙为 `assets/AssetPack.hap`
3. **RuntimeAssetManager**：从 Pack 按需加载（验证 Build 管线）

Editor **Play 仍用 `CopyTo`**（与 HazelEngine 一致）；AssetPack 面向发布/独立运行时。

---

## Prefab（缩小版）

### 新增类型

| 文件 | 职责 |
|------|------|
| `Scene/Components.h` | `IDComponent`、`PrefabComponent` |
| `Scene/Prefab.h/.cpp` | Prefab 资产（内部 `Ref<Scene>` + 根 Entity） |
| `Scene/Scene.cpp` | `Instantiate` / `DuplicateEntity` / `GetAssetList` |
| `Asset/AssetSerializer.cpp` | `PrefabSerializer` YAML 读写 |

### 工作流

```
Hierarchy 右键实体 → Create Prefab...
  → assets/prefabs/<Tag>.hprefab
  → registry 登记 Prefab 类型

Content Browser 双击 .hprefab
  → Scene::Instantiate(prefab)

实例带 PrefabComponent { PrefabID, EntityID }
Hierarchy 显示 [P] 前缀
```

### `.hprefab` 格式（YAML）

```yaml
Prefab:
  - Entity: <uuid>
    TagComponent: { Tag: ... }
    PrefabComponent: { Prefab: <handle>, Entity: <uuid> }
    TransformComponent: { Translation, Rotation, Scale }
    SpriteRendererComponent: { Color, Texture: <handle> }
```

扩展名对齐 HazelEngine：**`.hprefab`**（不是 `.hzprefab`）。

---

## AssetPack（缩小版）

### 新增模块

| 文件 | 职责 |
|------|------|
| `Core/Buffer.h` | 二进制缓冲 |
| `Serialization/FileStream*` | 文件流读写 |
| `Serialization/StreamReader/Writer*` | `WriteString` / `ReadBuffer` |
| `Serialization/AssetPackFile.h` | 索引结构（魔数 `HZAP`，Version=1） |
| `Serialization/AssetPackSerializer.*` | 序列化/反序列化索引 |
| `Serialization/AssetPack.*` | `CreateFromActiveProject` / `Load` |
| `Asset/AssetManager/RuntimeAssetManager.*` | 运行时 `GetAsset` → Pack |
| `Project::SetActiveRuntime` | 切换 Runtime 资产管理器 |

### `.hap` 布局（简化）

```
[Header: HZAP + version]
[Index placeholder → 末尾回填]
[App binary — 当前为空]
[Scene YAML blobs]
[Texture raw file bytes / Prefab YAML blobs]
[Index: scenes → assets offset/size/type]
```

### Build 流程

```
Build → Build Asset Pack
  → 扫描 registry 中所有 Scene
  → scene->GetAssetList() + Prefab 递归依赖
  → 写入 assets/AssetPack.hap
  → Load 验证索引
```

---

## 编辑器集成

| UI | 功能 |
|----|------|
| **Hierarchy** | Create Prefab / Update Prefab / `[P]` 标记 |
| **Content Browser** | Prefab 类型；双击实例化 |
| **Build 菜单** | Build Asset Pack |
| **Project 面板** | 显示上次 Build 状态 |

---

## 验收清单

- [ ] Hierarchy 右键创建 Prefab → 生成 `prefabs/*.hprefab` 与 registry 条目
- [ ] Content Browser 双击 Prefab → 场景中出现实例（带 `[P]`）
- [ ] Save Scene → 序列化 `PrefabComponent`
- [ ] Build Asset Pack → 生成 `assets/AssetPack.hap`
- [ ] 日志显示 Pack 索引 scene 数量

## 刻意未做

- 实体层级 / RelationshipComponent
- Apply/Revert Prefab 变体 UI
- 异步 Build、C# App binary 打包
- Play 模式从 AssetPack 加载（仍 CopyTo）
- Prefab 嵌套编辑窗口

## 下一模块（M7）

OpenGL 版 `SceneRenderer`：Viewport 3D 网格 + StaticMesh（见 M5/M6 讨论）。
