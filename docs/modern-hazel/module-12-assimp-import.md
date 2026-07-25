# M12 — Assimp 模型导入

> 对照：`HazelEngine` Content Browser 导入流程、`MeshSource` 资产  
> 依赖：vcpkg `assimp` 6.x

## 目标

1. 通过 **Assimp** 将 `.obj` / `.fbx` / `.gltf` / `.glb` / `.dae` 转为引擎内 **MeshSource**（`.hmesh`）
2. Content Browser：**Import Model** 工具栏按钮 + 右键导入
3. 单元测试验证 OBJ 几何解析

---

## 新增模块

| 文件 | 职责 |
|------|------|
| `Asset/AssimpMeshImporter.*` | Assimp 读模型 → `MeshSource` CPU 数据 |
| `Asset/MeshImportService.*` | 扩展名检测、输出路径、`ImportModelAsMeshSource` |
| `Asset/AssetSerializer.*` | `MeshSourceSerializer::SerializeToYAML` 公开给导入流程 |
| `tests/MeshImportTests.cpp` | Assimp 导入 + 扩展名检测 |

## Assimp 后处理标志

```cpp
aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords
| aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GlobalScale
```

多 SubMesh 合并为单一 `MeshSource`（顶点/索引拼接）。

## GPU 资源延迟创建

`MeshSource` 构造函数不再立即调用 `CreateGPUResources()`；首次 `GetVertexArray()` 时创建。  
这样单元测试在无 OpenGL 上下文时也能验证 Assimp 导入。

## Content Browser

- 工具栏 **Import Model**：文件对话框 → `assets/meshes/<相对路径>.hmesh`
- 右键模型文件：**Import as MeshSource**

## 构建注意

- `vcpkg.json` 声明 `assimp`（`default-features: false`）与 `box2d`
- Assimp 6 为 **动态库**；若运行时缺 DLL，需将 `assimp-vc143-mtd.dll` 复制到 exe 目录（vcpkg applocal 或手动）
- 若 vcpkg 编译报 `LNK1168`（DLL 被占用），删除 `buildtrees/assimp/.../bin/*.dll` 后单线程重试

## 验证

```powershell
cmake --build --preset=debug
ctest --preset=debug -R MeshImport
```

SampleProject：`assets/models/Cube.obj` 可供编辑器导入测试。
