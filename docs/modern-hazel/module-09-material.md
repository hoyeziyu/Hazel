# M9 — Material 资产（Albedo 纹理）

> 对照：`HazelEngine/Hazel/src/Hazel/Renderer/MaterialAsset.h`、`Asset/AssetSerializer.cpp`（MaterialAssetSerializer）  
> 实现：OpenGL 缩小版 Material + 纹理 Mesh Shader

## 目标

在 M8 StaticMesh 基础上，让 3D 网格可以使用 **Material 资产** 驱动外观：

1. **MaterialAsset**（`.hmaterial`）：YAML 存储 AlbedoColor、Metalness/Roughness（预留）、AlbedoMap 纹理 Handle
2. **MeshSource UV**：`.hmesh` 可选 `TexCoords`，兼容旧无 UV 网格
3. **Mesh.glsl**：AlbedoColor × 可选 AlbedoMap × 实体 Color Tint
4. **StaticMeshComponent.Material**：Inspector 下拉选择材质

---

## 资产格式

### `.hmaterial`（对齐 HazelEngine 子集）

```yaml
Material:
  AlbedoColor: [1, 1, 1]
  Metalness: 0
  Roughness: 0.5
  Emission: 0
  AlbedoMap: <texture-handle>
```

HazelEngine 完整版还有 NormalMap、MaterialFlags、Transparent 等字段；本模块刻意只实现 Albedo 路径。

### `.hmesh` 扩展

```yaml
MeshSource:
  Positions: [...]
  TexCoords: [[0,0], [1,0], ...]   # 可选，数量须与 Positions 一致
  Indices: [...]
```

SampleProject 的 `Cube.hmesh` 已改为 **24 顶点**（每面 4 点独立 UV），避免共享顶点无法贴图的问题。

---

## 新增 / 修改模块

| 文件 | 职责 |
|------|------|
| `Asset/MaterialAsset.h/.cpp` | 材质数据 + `GetAlbedoTexture()` |
| `Asset/AssetSerializer.*` | `MaterialAssetSerializer` |
| `Asset/AssetTypes.h` | `AssetType::Material` |
| `Asset/MeshSource.*` | 可选 TexCoords → GPU 顶点布局 |
| `Renderer/SceneRenderer.*` | `MeshMaterialData`、白纹理 fallback |
| `assets/shaders/Mesh.glsl` | 纹理 + AlbedoColor + Tint |
| `Scene/Components.h` | `StaticMeshComponent.Material` |
| `Scene/Scene.cpp` | RenderMeshes 解析 Material |
| `Scene/SceneSerializer.*` | 序列化 Material Handle |
| `Serialization/AssetPack.cpp` | Material → AlbedoMap 依赖打包 |
| `Hazelnut/.../SceneHierarchyPanel.cpp` | Material 下拉 |

---

## 渲染路径

```
StaticMeshComponent
  ├─ StaticMesh → MeshSource → VertexArray (Position + TexCoord)
  └─ Material → MaterialAsset
        ├─ AlbedoColor
        └─ AlbedoMap → TextureAsset → Texture2D
              ↓
SceneRenderer::SubmitMesh(..., MeshMaterialData)
  → Mesh.glsl: albedo = AlbedoColor * texture.rgb; output = albedo * ColorTint
```

无 Material 时：AlbedoColor = 白，仅用 `Color` Tint（与 M8 行为一致）。

---

## 验收

- [x] SampleProject Cube 使用 `materials/Checkerboard.hmaterial` 显示棋盘格
- [x] Content Browser 识别 `.hmaterial` 为 Material
- [x] Properties 可切换 Material
- [x] Save Scene → `.hazel` 含 `Material: <handle>`
- [x] AssetPack 打包 Material + AlbedoMap 纹理依赖
- [x] 单元测试 `MaterialTests.cpp`

---

## 性能备注

`HazelBench` 新增 `BM_MaterialAssetLookup`，用于对比 Material 字段读取开销（当前为纯 CPU 数据，无 GPU 绑定）。

完整 Draw Call 统计留待 M10（SceneRenderer Stats 面板）。

---

## 刻意未做

- PBR 光照（Metalness/Roughness 仅存储，不参与 Shader）
- NormalMap / 透明材质 / MaterialTable
- Material Editor 专用面板
- Assimp 自动 UV

---

## 下一模块（M10）

建议：**Directional Light + 简单 Phong/Blinn** 或 **Box2D 物理**，对照 HazelEngine `DirectionalLightComponent` / `Physics2D`。

详见 [ROADMAP.md](./ROADMAP.md)。
