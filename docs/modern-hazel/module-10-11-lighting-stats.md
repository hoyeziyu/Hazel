# M10 — 方向光 + Blinn-Phong 光照

> 对照：`HazelEngine/Hazel/src/Hazel/Scene/Components.h`（DirectionalLightComponent）、`Scene.cpp` 光照收集  
> 实现：OpenGL 前向 Blinn-Phong + Mesh 法线

## 目标

1. **DirectionalLightComponent**：Radiance、Intensity；方向由 Transform 推导（与 HazelEngine 一致：`-(R * vec3(1,0,0))`）
2. **MeshSource 法线**：可选 YAML `Normals`；缺失时 `GenerateSmoothNormals()`
3. **Mesh.glsl**：环境光 + 方向光 Diffuse + Blinn-Phong Specular
4. **SceneEnvironmentData**：相机位置 + 首盏方向光 → SceneRenderer Uniform

## 光照公式（缩小版）

```
albedo = AlbedoColor * (texture?)
N = normalize(normal)
L = normalize(-LightDirection)
V = normalize(CameraPos - WorldPos)
H = normalize(L + V)
lighting = 0.08 * albedo + Radiance * Intensity * (diffuse * albedo + 0.25 * spec)
output = lighting * ColorTint
```

## 新增 / 修改

| 文件 | 职责 |
|------|------|
| `Renderer/SceneEnvironment.h` | `SceneEnvironmentData`、`SceneRendererStats` |
| `Scene/Components.h` | `DirectionalLightComponent` |
| `Asset/MeshSource.*` | 法线生成与 GPU 布局 Position+Normal+TexCoord |
| `Renderer/SceneRenderer.*` | 环境 Uniform、`BeginScene(vp, env)` |
| `Scene/Scene.cpp` | `BuildSceneEnvironment()` |
| `Hazelnut/.../SceneHierarchyPanel.cpp` | Add Directional Light + Inspector |

SampleProject `Main.hazel` 已添加 **Directional Light** 实体。

## 验收

- [x] 有方向光时 Cube 明暗随法线变化
- [x] 无方向光时仅环境光（`HasDirectionalLight = false`）
- [x] Play 模式光照与 Edit 一致
- [x] 场景序列化含 DirectionalLightComponent

---

# M11 — SceneRenderer 统计

> 对照：HazelEngine `RendererStats` / 编辑器 Stats 面板

## 实现

- `SceneRendererStats`：`DrawCalls`、`TriangleCount`
- 每次 `SubmitMesh` 累加
- Hazelnut **Stats** 面板显示 SceneRenderer 区段（与 Renderer2D 并列）

## 验收

- [x] Stats 面板显示 3D Draw Calls / Triangles
- [x] 每帧 `BeginScene` 重置计数

## 下一模块

[M12 Assimp 模型导入](./module-12-assimp.md)
