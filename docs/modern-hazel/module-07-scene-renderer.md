# M7 — SceneRenderer（3D 视口）

> 对照：`HazelEngine/Hazel/src/Hazel/Renderer/SceneRenderer.*`  
> 实现：OpenGL 缩小版 Forward + Grid + 内置 Cube；保留 Renderer2D 绘制 Sprite

## 目标

在 Edit / Play 视口中叠加 **3D 渲染**：

1. **SceneRenderer**：Forward 管线，绘制带 `MeshRendererComponent` 的实体（内置立方体）
2. **Grid**：XZ 地面网格，View 菜单可 Toggle
3. **Renderer2D**：3D 之后叠加 Sprite（深度测试关闭，避免被网格遮挡）

EditorLayer **仍拥有 FBO**；SceneRenderer 绘制到当前绑定的 Framebuffer（含 depth attachment）。

---

## 新增类型

| 文件 | 职责 |
|------|------|
| `Renderer/SceneRenderer.h/.cpp` | `Init` / `BeginScene` / `SubmitMesh` / `RenderGrid` |
| `Scene/Components.h` | `MeshRendererComponent { Color, Visible }` |
| `Hazelnut/assets/shaders/Mesh.glsl` | 简单 forward（`u_ViewProjection` + `u_Transform` + `u_Color`） |
| `Hazelnut/assets/shaders/Grid.glsl` | 参考 HazelEngine Grid，改为 OpenGL `#type` 格式 |

---

## 渲染顺序

```
EditorLayer::OnUpdate
  FBO Bind → Clear
  Edit: Scene::OnRenderEditor(SceneRenderer, EditorCamera, showGrid)
  Play: Scene::OnUpdateRuntime → Scene::OnRenderRuntime(SceneRenderer, showGrid)
  FBO Unbind
```

`Scene::OnRenderEditor` / `OnRenderRuntime` 内部：

```
1. SceneRenderer BeginScene → RenderMeshes → RenderGrid → EndScene
2. Prepare2DOverlay (glDisable depth)
3. Renderer2D BeginScene → RenderSprites → EndScene
4. RestoreAfter2D
```

Play 模式脚本更新与渲染分离：`OnUpdateRuntime` 只跑 NativeScript，`OnRenderRuntime` 用 Primary Camera 走同样 3D+2D 路径。

---

## Editor 集成

| 位置 | 改动 |
|------|------|
| `EditorLayer` | `m_SceneRenderer`、`m_ShowGrid`；View → Show Grid |
| `SceneHierarchyPanel` | Add Component → Mesh Renderer；Properties 编辑 Color / Visible |
| `SceneSerializer` | 序列化 `MeshRendererComponent` |
| `SampleProject/Main.hazel` | 默认 Cube 实体（y=0.5，半高贴地） |

---

## 验收

- [ ] Edit Viewport 显示地面 Grid（View → Show Grid 可关）
- [ ] 带 Mesh Renderer 的实体显示立方体
- [ ] 原有 Sprite 仍正常（绿/红方块）
- [ ] Play 模式 Primary Camera 下 3D + Sprite 正常
- [ ] 深度测试：Cube 被 Grid 正确遮挡关系；2D Sprite 叠在最上层

---

## 后续（M9+）

- Material 资产、纹理 Mesh Shader
- 多 Pass / 阴影 / PBR（HazelEngine Vulkan 完整管线）
- SceneRenderer 统计（Draw Calls）写入 Stats 面板
