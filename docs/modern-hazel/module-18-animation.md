# M18 — AnimationGraph 缩小版

## 目标

实现 LD51 玩家动画所需的最小链路：**Assimp 导入骨骼/权重/动画 → `.hanimc` 控制器 → `AnimationComponent` 状态播放 → 蒙皮网格渲染**，并暴露与 LD51 `PlayerBase` 兼容的 C# API。

## 与 HazelEngine 的差异

| 能力 | HazelEngine | 本模块 |
|------|-------------|--------|
| 动画压缩 | ACL | 关键帧（Assimp 导入后直接采样） |
| 控制器 | `.hanimgraph` + NodeGraph | `.hanimc` 多状态列表 |
| C# API | AnimationGraph 输入 | `StateIndex` / `AnimationTime` / `IsAnimationPlaying` |
| 渲染 | Vulkan SSBO | OpenGL uniform `u_BoneTransforms[100]` |

## 核心模块

| 类型 | 路径 | 说明 |
|------|------|------|
| 骨架 | `Hazel/Animation/Skeleton.*` | 骨骼层级与 rest pose |
| 剪辑 | `Hazel/Animation/AnimationClip.*` | 关键帧采样 |
| 运行时 | `Hazel/Animation/AnimationSystem.*` | 更新动画、计算蒙皮矩阵 |
| 导入 | `AssimpMeshImporter` + `AssimpAnimationImporter` | 一次导入几何 + 骨骼 + 动画 |
| 资产 | `.hskel` / `.hanim` / `.hanimc` | YAML 引用 MeshSource 与状态 |
| 组件 | `AnimationComponent` / `SkinnedMeshComponent` | 播放控制 + 蒙皮绘制 |
| Shader | `assets/shaders/SkinnedMesh.glsl` | 4-bone 线性混合蒙皮 |

## 资产格式

### `.hskel`

```yaml
Skeleton:
  MeshSource: 4256841120731152137
```

### `.hanim`

```yaml
Animation:
  AnimationSource: 4256841120731152137
  SkeletonSource: 4256841120731152137
  AnimationIndex: 0
```

### `.hanimc`（LD51 兼容）

```yaml
AnimationController:
  SkeletonAsset: 15841319722218504709
  States:
    - AnimationAsset: 9877617479002501151
      AnimationIndex: 0
      Loop: true
    - AnimationAsset: 9877617479002501151
      AnimationIndex: 1
      Loop: true
```

## 组件字段

**AnimationComponent**（与 LD51 prefab 对齐）：

- `AnimationController` — `.hanimc` 资产句柄
- `BoneEntities` — 与 skeleton 骨骼一一对应的子实体 UUID
- `StateIndex` / `AnimationTime` / `IsAnimationPlaying` / `PlaybackSpeed`

**SkinnedMeshComponent**：

- `StaticMesh` + `Material` + `BoneEntities`
- 使用 `MeshSource::GetSkinnedVertexArray()` 与 bone 矩阵绘制

## 蒙皮数据持久化

`.hmesh` 可含 `SourceModel` 相对路径；加载时若缺少 rig 数据，会从源 gltf/fbx **重新导入**骨骼与动画。

## C# API（LD51 PlayerBase）

```csharp
m_AnimationController.StateIndex = animationState;
m_AnimationController.AnimationTime = 0.0f;
m_AnimationController.IsAnimationPlaying = true;
```

InternalCalls：`AnimationComponent_Get/SetStateIndex`、`Get/SetAnimationTime`、`Get/SetIsAnimationPlaying`。

## 自测清单

```powershell
cmake --preset=windows-msvc-debug
cmake --build --preset=debug --target Hazelnut
ctest --preset=debug
```

- [x] Assimp 导入带骨骼 gltf/fbx
- [x] `.hanimc` / `.hskel` / `.hanim` 序列化
- [x] Play 模式动画采样 + 蒙皮渲染
- [x] ctest 含 `AnimationTest.*`（24/24）

## 后续扩展

- Root motion 掩码（LD51 `RootTranslationMask`）
- AnimationGraph 编辑器
- ACL 压缩与更大 bone 数 SSBO 批处理
