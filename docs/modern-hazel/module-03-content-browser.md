# M3 — Content Browser

> 对照：`HazelEngine/Hazelnut/src/Panels/ContentBrowserPanel.*`  
> 实现：`Hazelnut/src/Panels/ContentBrowserPanel.*`

## StudioCherno 怎么做

- 绑定 `Project::GetAssetDirectory()` 与 **AssetManager**（`AssetHandle`、缩略图、拖拽 payload）
- 左树右网格、多选、复制/重命名/删除、搜索、面包屑历史
- `OnProjectChanged` 重建目录树；双击 Scene 资源 → `OpenScene(metadata)`

## 我们的缩小版

### 数据模型

- **无 AssetManager**（M5 再做），直接用 `std::filesystem`
- 根目录：`Project::GetAssetDirectory()`（即 `{ProjectRoot}/assets`）
- 当前路径：相对 assets 的 `m_CurrentRelativePath`

### UI

| 区域 | 功能 |
|------|------|
| 顶栏 | Up / Refresh / 当前路径 |
| 左侧 | Assets 目录树（单击切换当前目录） |
| 右侧 | 文件表（Name + Type） |
| 交互 | 单击选中；**双击文件夹**进入；**双击 `.hazel`** 打开场景 |

### 文件类型标签

| 扩展名 | Type |
|--------|------|
| `.hazel` | Scene（双击打开） |
| `.png/.jpg` | Texture |
| `.glsl` | Shader |
| `.ttf` | Font |
| 目录 | Folder |

隐藏以 `.` 开头的条目。

### EditorLayer 集成

```cpp
m_ContentBrowserPanel.SetSceneActivatedCallback([this](const auto& path) { LoadScene(path); });
// OpenProject → OnProjectChanged(project)
// CloseProject → OnProjectChanged(nullptr)
```

## 验收清单

- [ ] 打开 SampleProject，出现 **Content Browser**
- [ ] 左侧展开 `scenes/`，右侧见 `Main.hazel`
- [ ] 双击 `Main.hazel` 加载场景
- [ ] Up / Refresh 可用
- [ ] 切换 / 关闭 Project 时 Browser 同步更新或隐藏

## 刻意未做

- AssetRegistry / 缩略图 / 拖拽 / 多选 / 文件 CRUD
- `ContentBrowserItem` 独立类（参考版 600+ 行）

## 下一模块（M5）

资产系统：AssetManager、`.hzr` 注册表、Content Browser 与 Handle 对接。
