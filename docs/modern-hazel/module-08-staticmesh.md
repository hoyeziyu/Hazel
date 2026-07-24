# M8 — StaticMesh 资产

> 对照：`HazelEngine/Hazel/src/Hazel/Renderer/Mesh.h`、`Asset/MeshSerializer.*`  
> 实现：OpenGL 缩小版 MeshSource + StaticMesh + StaticMeshComponent

## 目标

在 M7 SceneRenderer 基础上，用 **资产引用** 替换内置 Cube：

1. **MeshSource**（`.hmesh`）：YAML 存储 Positions / Indices，加载时创建 `VertexArray`
2. **StaticMesh**（`.hsm`）：YAML 引用 MeshSource Handle（对齐 HazelEngine 格式）
3. **StaticMeshComponent**：场景实体引用 StaticMesh，SceneRenderer 绘制真实网格

保留 **MeshRendererComponent** 作为无资产时的内置 Cube 后备。

---

## 资产格式

### `.hmesh`（MeshSource）

```yaml
MeshSource:
  Positions:
    - [-0.5, -0.5, -0.5]
    - [0.5, -0.5, -0.5]
    ...
  Indices: [0, 1, 2, 2, 3, 0, ...]
```

### `.hsm`（StaticMesh）

```yaml
Mesh:
  MeshSource: <handle>
  SubmeshIndices: []
```

---

## 新增模块

| 文件 | 职责 |
|------|------|
| `Asset/MeshSource.h/.cpp` | 几何数据 + GPU `VertexArray` |
| `Asset/StaticMesh.h/.cpp` | 引用 MeshSource Handle |
| `Asset/AssetSerializer.*` | `MeshSourceSerializer` / `StaticMeshSerializer` |
| `Asset/AssetTypes.h` | `MeshSource`、`StaticMesh` 类型 |
| `Scene/Components.h` | `StaticMeshComponent { StaticMesh, Color, Visible }` |
| `Scene/Scene.cpp` | `RenderMeshes` 从 StaticMesh → MeshSource 取 VA |
| `Scene/SceneSerializer.*` | 序列化 StaticMeshComponent |
| `Serialization/AssetPack.cpp` | StaticMesh 依赖自动打包 MeshSource |

---

## 编辑器

- **Add Component → Static Mesh**
- Properties：**Mesh** 下拉（registry 中所有 `.hsm`）、Color、Visible
- Content Browser：`.hmesh` / `.hsm` 显示类型 MeshSource / StaticMesh
- SampleProject：`meshes/Cube.hmesh` + `Cube.hsm`，Main 场景 Cube 实体已绑定

---

## 渲染路径

```
StaticMeshComponent.StaticMesh
  → GetAsset<StaticMesh>
  → GetAsset<MeshSource>(meshSourceHandle)
  → VertexArray
  → SceneRenderer::SubmitMesh(vertexArray, transform, color)

无效 Handle / 加载失败 → 回退 SceneRenderer 内置 Cube
MeshRendererComponent（遗留）→ 始终内置 Cube
```

---

## 验收

- [ ] SampleProject Main 场景 Cube 使用 `meshes/Cube.hsm` 显示
- [ ] Content Browser 识别 MeshSource / StaticMesh 类型
- [ ] Properties 可切换 Static Mesh 资产
- [ ] Save Scene → `.hazel` 含 `StaticMesh: <handle>`
- [ ] Build AssetPack 包含 StaticMesh + MeshSource 依赖

---

## 刻意未做

- Assimp / FBX 导入、Submesh、MaterialTable
- 缩略图、Content Browser 双击预览网格
- MeshRendererComponent 移除（仍作简易 Cube 后备）

## 下一模块（M9）

可选：Material 资产、纹理 Mesh Shader，或 Script 组件基础。
