# M13 — Box2D Physics2D

> 对照：`HazelEngine/Hazel/src/Hazel/Physics2D`、`Scene.cpp` Play 模式 Box2D 初始化  
> 依赖：vcpkg **Box2D 3.x**（API 与 HazelEngine 的 Box2D 2.x 不同）

## 目标

1. **RigidBody2DComponent** + **BoxCollider2DComponent**
2. Play 模式：`OnRuntimeStart` 创建 Box2D 世界与刚体/形状，`OnUpdateRuntime` 步进并回写 `TransformComponent`
3. 场景 YAML 序列化 / Inspector 编辑

---

## 组件

| 组件 | 字段 |
|------|------|
| `RigidBody2DComponent` | BodyType (Static/Dynamic/Kinematic)、FixedRotation、GravityScale、阻尼 |
| `BoxCollider2DComponent` | Offset、Size（半尺寸，与 HazelEngine 一致）、Density、Friction |

运行时 Box2D id 存为 `uint64_t` 句柄（不参与序列化）。

## Physics2DScene

`Hazel/Physics2D/Physics2DScene.cpp` 封装 Box2D 3 API：

- `Init` → `b2CreateWorld`、创建 body/shape
- `Step` → 固定 16ms 子步、`b2World_Step`、同步 Dynamic/Kinematic 实体 Translation/Rotation.z
- `Shutdown` → `b2DestroyWorld`、清空运行时句柄

重力：`{0, -9.81}`。

## 与 HazelEngine 差异

| 项 | HazelEngine | 本仓库 |
|----|-------------|--------|
| Box2D 版本 | 2.x (`b2World*`) | 3.x (`b2WorldId`) |
| CircleCollider2D | 有 | 暂未实现 |
| Raycast API | `Physics2D::Raycast` | 后续模块 |

## 验证

```powershell
ctest --preset=debug -R Physics2D
```

- `DynamicBodyFallsUnderGravity`：5m 高处 Dynamic 盒子在 0.5s 内下落
- `SerializeAndDeserializeRoundTrip`：YAML 往返

## Hazelnut 使用

1. 实体 Add Component → **Rigid Body 2D** + **Box Collider 2D**
2. 地面：Static + 大 Scale 的 Box Collider
3. Play：Dynamic 物体会受重力并更新 Transform（2D 使用 X/Y）
