# Scene — ECS 游戏世界

基于 EnTT 的实体–组件–系统：`Scene` 持有 `registry`，`Entity` 是带组件操作的轻量句柄。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 根文档 §3

---

## 目录与职责

| 文件 | 职责 |
|------|------|
| `Scene.h/.cpp` | 世界容器；`OnUpdate` 跑脚本 + 2D 渲染 |
| `Entity.h/.cpp` | `AddComponent` / `GetComponent` / `HasComponent` |
| `Components.h` | `TransformComponent`、`SpriteRendererComponent`、`CameraComponent`、`TagComponent`、`NativeScriptComponent` |
| `SceneCamera.h/.cpp` | 场景相机（正交/透视） |
| `ScriptableEntity.h` | 可挂载 C++ 脚本基类 |
| `SceneSerializer.h/.cpp` | `.hazel` YAML 序列化（yaml-cpp） |

---

## 与 Layer 的关系

```
Layer（应用层）              Scene（世界层）
  持有 Ref<Scene>              不知道 LayerStack
  OnUpdate → m_Scene->OnUpdate   只管 registry + 渲染提交
  OnEvent → OnViewportResize     不注册到事件总线
```

Scene **不继承 Layer**；必须由 Layer（或 Editor）持有并每帧调用。

---

## Scene::OnUpdate 流程

```
1. NativeScriptComponent
     each → InstantiateScript / OnCreate / OnUpdate(ts)

2. 找 Primary CameraComponent
     → Renderer2D::BeginScene(camera, transform)

3. group<Transform, SpriteRenderer>
     → Renderer2D::DrawQuad(transform, color)   // 只写 staging

4. Renderer2D::EndScene()   // 整场景通常 1 Draw Call
```

---

## Entity / 组件约定

- 创建：`CreateEntity(name)` 自动加 `TransformComponent` + `TagComponent`。
- 加组件：`entity.AddComponent<T>()` → 触发 `Scene::OnComponentAdded<T>` 模板特化。
- EnTT 头文件：`#include <entt/entt.hpp>`（不是 `entt.hpp`）。
- 脚本绑定：`nativeScript.Bind<MyScript>()`，`MyScript : ScriptableEntity`。

---

## 序列化

- 编辑器：`SceneSerializer::Serialize/Deserialize` → `assets/scenes/*.hazel`。
- 运行时格式：`SerializeRuntime` / `DeserializeRuntime`（预留）。

---

## 修改时注意

- 新组件类型：在 `Components.h` 声明 + `Scene.cpp` 增加 `OnComponentAdded` 特化。
- `Entity.h` 与 `Scene.h` 互相 friend；Include 注意循环依赖。
- 单元测试可测 Scene/Entity 纯逻辑，**勿**在无 GL 上下文时调 `Renderer2D::BeginScene`。
