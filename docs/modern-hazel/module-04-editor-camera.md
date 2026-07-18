# M4 — Editor Camera

> 对照：`HazelEngine/Hazel/src/Hazel/Editor/EditorCamera.*`  
> 实现：`Hazel/src/Hazel/Editor/EditorCamera.*`、`Hazelnut/src/EditorLayer.*`

## StudioCherno 怎么做

- Edit 模式 Viewport 用 **EditorCamera**（透视 Arcball），与场景 **Primary Camera** 实体解耦
- Play 模式仍走 `Scene::OnUpdateRuntime` + 场景主相机
- Gizmo 使用 EditorCamera 的 view / projection（透视 `SetOrthographic(false)`）

## 我们的缩小版

### 引擎：`EditorCamera`

| 操作 | 输入 |
|------|------|
| 旋转 | Alt + 左键拖拽 |
| 平移 | Alt + 中键拖拽 |
| 缩放 | Alt + 右键拖拽，或滚轮 |
| 激活 | `SetActive(true)` 时响应（EditorLayer 绑定 Viewport focus + hover） |

- 透视投影：`glm::perspective`，默认 FOV 45°
- 提供 `GetViewMatrix()` / `GetProjectionMatrix()` / `GetViewProjection()`

### 渲染路径分离

```
Edit:
  EditorLayer → m_EditorScene->OnRenderEditor(m_EditorCamera)
            → Renderer2D::BeginScene(viewProjection)
            → 只画 Sprite，不用 Primary Camera

Play:
  EditorLayer → m_RuntimeScene->OnUpdateRuntime(ts)
            → Scene 内 Primary Camera + BeginScene(camera, transform)
```

`OnUpdateEditor` 不再渲染（仅留给未来 Edit 逻辑）。

### EditorLayer 变更

- 移除 `OrthographicCameraController`，改为 `EditorCamera m_EditorCamera`
- Viewport resize → `SetViewportSize`
- Gizmo 矩阵来自 `m_EditorCamera`（非场景相机）
- `OrthographicCameraController` 仍保留在引擎中供 Sandbox 使用

### Renderer2D

新增重载：

```cpp
static void BeginScene(const glm::mat4& viewProjection);
```

供 EditorCamera 直接传入合成矩阵。

## 验收清单

- [ ] Edit 模式：Alt+鼠标可 orbit / pan / zoom Viewport
- [ ] 移动 EditorCamera **不会**改变 Hierarchy 里 Camera 实体的 Transform
- [ ] Gizmo 与 Sprite 对齐（透视下 W/E/R 正常）
- [ ] Play 模式：仍用场景 Primary Camera，与 Edit 相机无关
- [ ] Stop 回到 Edit 后 EditorCamera 状态保留

## 刻意未做

- EditorCamera 序列化 / 每场景独立相机状态
- 聚焦选中实体（Frame Selected）
- 2D 正交 Editor 模式切换

## 下一模块（M5）

资产系统：AssetManager、`.hzr` 注册表、Content Browser 与 Handle 对接。
