#pragma once

#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/Camera.h"
#include "Texture.h"
#include "SubTexture2D.h"

namespace Hazel {
	// 2D 合批渲染器（学习入口）
	//
	// 一批的生命周期：
	//   BeginScene()  → 重置 staging，设相机
	//   DrawQuad*()   → 只写 CPU 缓冲（合批）
	//   EndScene()    → Upload VBO + Flush（正常结束，+1 DrawCall）
	//   FlushAndReset → 缓冲/slot 满时中途拆批（见 Renderer2D.cpp 文件头）
	//
	// 与 Renderer::Submit 对比：Submit 每物体 1 DrawCall；Renderer2D 整批通常 1 DrawCall。
	class Renderer2D
	{
		// 全部是 static 方法；实际状态在 Renderer2D.cpp 的 s_Data 单例里
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);   // 设 u_ViewProjection，重置 staging
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();   // 上传 VBO + Flush（至少 1 Draw Call）

		static void Flush();      // 绑纹理 slot + glDrawElements；Stats.DrawCalls++

		// Primitives — Draw* 只写 CPU 缓冲，不触发 draw
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		
		// texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// rotation is in radians 输入弧度
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Stats：QuadCount = 提交 Quad 数；DrawCalls = Flush 次数（合批后通常远小于 QuadCount）
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void FlushAndReset();   // Quad/纹理 slot 满时拆批
	};
}