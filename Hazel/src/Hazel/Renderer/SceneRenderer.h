#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SceneEnvironment.h"

#include <glm/glm.hpp>

namespace Hazel {

	struct MeshMaterialData
	{
		glm::vec4 ColorTint{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec3 AlbedoColor{ 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> AlbedoTexture;
	};

	class EditorCamera;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;

		void Init();

		void SetGridEnabled(bool enabled) { m_GridEnabled = enabled; }
		bool IsGridEnabled() const { return m_GridEnabled; }

		void BeginScene(const EditorCamera& camera, const SceneEnvironmentData& environment);
		void BeginScene(const glm::mat4& viewProjection, const SceneEnvironmentData& environment);
		void EndScene();

		void SubmitMesh(const glm::mat4& transform, const glm::vec4& color);
		void SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const glm::vec4& color);
		void SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const MeshMaterialData& material);
		void SubmitSkinnedMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const MeshMaterialData& material, const std::vector<glm::mat4>& boneMatrices);

		const Ref<VertexArray>& GetDefaultCubeMesh() const { return m_CubeVertexArray; }
		const SceneRendererStats& GetStats() const { return m_Stats; }
		void RenderGrid();

		void Prepare2DOverlay();
		void RestoreAfter2D();

	private:
		void ApplyEnvironmentUniforms();

		Ref<Shader> m_MeshShader;
		Ref<Shader> m_SkinnedMeshShader;
		Ref<Shader> m_GridShader;
		Ref<Texture2D> m_WhiteTexture;
		Ref<VertexArray> m_CubeVertexArray;
		Ref<VertexArray> m_GridVertexArray;

		glm::mat4 m_ViewProjection = glm::mat4(1.0f);
		SceneEnvironmentData m_Environment;
		SceneRendererStats m_Stats;
		bool m_GridEnabled = true;
	};

}
