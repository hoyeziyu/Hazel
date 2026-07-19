# M5 — 资产系统

> 对照：`HazelEngine/Hazel/src/Hazel/Asset/*`  
> 实现：`Hazel/src/Hazel/Asset/*`、`Hazelnut` Content Browser / Properties 集成

## 管理方案（批注）

HazelEngine 的资产系统核心是 **「路径与引用分离」**：

```
磁盘文件 (assets/...)
    ↕ ImportAsset（扫描 / 拖入）
AssetRegistry.hzr  ←→  AssetMetadata { Handle, FilePath, Type }
    ↕ GetAsset(handle) 按需加载
内存对象 Ref<Asset>（如 TextureAsset）
    ↕ 组件只存 Handle
Scene / Component 序列化字段（uint64 AssetHandle）
```

### 为什么不用路径直接引用？

| 直接存路径 | 用 AssetHandle |
|-----------|----------------|
| 重命名/移动文件会断引用 | 注册表可更新 `FilePath`，Handle 不变 |
| 场景 YAML 里是字符串路径，难统一管理 | 场景里只存 ID，语义清晰 |
| 无法做类型校验、依赖、打包 | 为 Runtime AssetPack 打基础 |

### 三层职责

1. **注册层（`.hzr`）**  
   - 文件：`assets/AssetRegistry.hzr`  
   - 内容：`Handle` + 相对路径 + `Type`  
   - **Import 只登记，不读像素/不解析场景**

2. **加载层（EditorAssetManager）**  
   - `GetAsset(handle)` 首次调用时才 `Texture2D::Create(...)`  
   - 结果缓存在 `m_LoadedAssets`

3. **使用层（组件 / 编辑器）**  
   - `SpriteRendererComponent::Texture` 存 `AssetHandle`  
   - Content Browser 显示 Type；Properties 用 Handle 选纹理  
   - 场景 `.hazel` 序列化 `Texture: <uint64>`

### 与 M3 Content Browser 的关系

- **M3**：`std::filesystem` 扫目录，双击路径打开场景  
- **M5**：Refresh 时对文件 `ImportAsset`；Type 列来自 registry；双击 Scene 仍 `LoadScene`，但路径来自 `GetFileSystemPath(handle)`

---

## StudioCherno 完整版还有哪些

- 异步 `EditorAssetSystem`、热重载、缩略图、拖拽 payload  
- Mesh / Material / Prefab / Script 等 20+ 类型  
- `RuntimeAssetManager` + `AssetPack`（Play 发布）  
- 文件监视、重命名/删除同步 registry  

## 我们的缩小版（M5）

### 新增引擎模块

| 文件 | 职责 |
|------|------|
| `Core/UUID.*` | 64-bit ID → `AssetHandle` |
| `Asset/Asset*.h` | 基类、Metadata、Registry |
| `Asset/AssetExtensions.h` | `.hazel`→Scene, `.png`→Texture … |
| `Asset/TextureAsset.h` | 包装 `Ref<Texture2D>` |
| `Asset/AssetImporter.*` | 按 Type 加载（当前仅 Texture） |
| `Asset/AssetManager/EditorAssetManager.*` | 同步版注册表 + 加载 |
| `Asset/AssetManager.h/.cpp` | 静态门面 → `Project::GetAssetManager()` |

### Project 生命周期

```cpp
Project::SetActive(project);
  → CreateRef<EditorAssetManager>()
      → LoadAssetRegistry()
      → ScanAndRegisterAssets()  // 递归 Import + 写 .hzr

Project::ClearActive() / 析构
  → EditorAssetManager::Shutdown() → WriteRegistryToFile()
```

### `.hzr` 示例

```yaml
Assets:
  - Handle: 123456789012345678
    FilePath: scenes/Main.hazel
    Type: Scene
  - Handle: 987654321098765432
    FilePath: textures/Checkerboard.png
    Type: Texture
```

### Scene 集成

- `SpriteRendererComponent` 增加 `AssetHandle Texture`  
- `SceneSerializer` 读写 `Texture` 字段  
- `Scene::RenderSprites`：`GetAsset<TextureAsset>` 有纹理则贴图，否则纯色 Quad  

### 编辑器 UI

- **Content Browser**：隐藏 `AssetRegistry.hzr`；Type 来自 registry  
- **Properties / Sprite Renderer**：Texture 下拉（registry 中所有 Texture）  
- **Project 面板**：显示 registry 路径与条目数  

---

## 验收清单

- [ ] 打开 SampleProject 后生成 `assets/AssetRegistry.hzr`  
- [ ] Content Browser 中 Scene/Texture 类型正确  
- [ ] Properties 给 Sprite 指定 `textures/Checkerboard.png`  
- [ ] Save Scene → `.hazel` 含 `Texture: <handle>`  
- [ ] 重开项目后 Handle 不变、纹理仍显示  
- [ ] Close Project 时 registry 写回磁盘  

## 刻意未做

- 异步加载、AssetPack、RuntimeAssetManager  
- 缩略图 / 拖拽 / CRUD / 文件监视  
- Mesh、Material、Font 等资源加载（Font 仍走 ImGui 路径）  

## 下一模块（M6）

可选方向：Play 模式 AssetPack、或脚本/Prefab 引用同一 Handle 体系。

## 下一模块（M7）

OpenGL SceneRenderer（Viewport 3D 渲染管线缩小版）。
