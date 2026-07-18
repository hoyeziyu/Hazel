# Renderer — 渲染抽象与 2D 合批

渲染 API 抽象、资源工厂、2D 合批（`Renderer2D`）与早期逐物体路径（`Renderer::Submit`）。

上级索引：**[`../../../AGENTS.md`](../../../AGENTS.md)** · 平台实现 **[`../../Platform/AGENTS.md`](../../Platform/AGENTS.md)** · 根文档 §3–§4

---

## 目录结构

```
Renderer/
├── Renderer.h/.cpp          # Init、旧 Submit 路径
├── Renderer2D.h/.cpp        # 2D 合批（主路径）★
├── RenderCommand.h/.cpp     # Clear / Viewport / DrawIndexed 门面
├── RendererAPI.h/.cpp       # 抽象 + GetAPI() → OpenGL
├── GraphicsContext.h/.cpp   # 上下文工厂 → OpenGLContext
├── Camera.h                 # 投影基类
├── OrthographicCamera*      # 2D 正交相机
├── OrthographicCameraController*
├── Shader.h/.cpp            # + ShaderLibrary
├── Buffer.h/.cpp            # VertexBuffer / IndexBuffer + Layout
├── VertexArray.h/.cpp
├── Texture.h/.cpp           # Texture2D 工厂
├── SubTexture2D*            # 图集 UV
└── Framebuffer.h/.cpp       # 离屏渲染（编辑器 Viewport）
```

---

## 分层调用

```
应用 Layer / Scene
  → Renderer2D::BeginScene / DrawQuad / EndScene   （合批）
  或 Renderer::Submit                                 （无合批，教程早期）
  → RenderCommand::DrawIndexed
  → RendererAPI (OpenGLRendererAPI)
  → glDrawElements
```

资源创建：`X::Create(...)` 内 `switch (Renderer::GetAPI())` → `OpenGL*` 实现。

---

## Renderer2D 合批流程（当前主路径）

**批处理本质：** 把所有 Quad 批量合并到一块 VBO + IBO，当作一个几何体，`glDrawElements` 只调用一次。

```
BeginScene(camera)     设 u_ViewProjection，重置 CPU staging
  DrawQuad × N         只写 CPU 缓冲（staging），不 draw
EndScene()             staging → GPU VBO（stream），Flush()
  Flush()              绑纹理 slot + 一次 DrawIndexed  → Stats.DrawCalls++
```

### 动态批处理（VBO / IBO 思路，对应 Renderer2D + OpenGLBuffer）

| 缓冲 | Init（分配/固定） | 每帧（渲染） |
|------|-------------------|--------------|
| **VBO** | `glBufferData(size, nullptr, GL_DYNAMIC_DRAW)` 只分配容量 | `SetData` → `glBufferSubData` 从 CPU 复制本帧顶点 |
| **IBO** | 预生成「最多 MaxQuads 个 Quad」的索引模式（静态） | `DrawIndexed(..., indexCount)` 控制本批实际画几个 Quad |

**粒子系统：** 四边形每帧运动 → 顶点每帧变 → VBO 必须 Dynamic + 每帧 stream；IBO 模式可不变，只改 `indexCount`。

**拆批（FlushAndReset）：** Quad ≥ 20000，或纹理种类 ≥ 32，或正常 `EndScene`。

详见 `Renderer2D.cpp` 文件头注释；shader 见 `assets/shaders/Texture.glsl`。

---

## Renderer::Submit（对比）

每调用一次：`Bind shader` + `u_ViewProjection` + `u_Transform` + `DrawIndexed` = **1 Draw Call**。Transform 在 GPU uniform；Renderer2D 在 CPU 烘焙进顶点。

---

## 修改时注意

- 渲染基类解绑方法名：**`UnBind`**（大写 B）。
- OpenGL 头顺序：先 `glad.h` 再 `glfw3.h`（在 Platform 层）。
- 新 `.cpp` 放 `Hazel/src/` 会被 GLOB 自动收录，**无需改 CMake**。
- 改 `MaxTextureSlots` 须同步 `Texture.glsl` 的 `u_Textures[N]`。
