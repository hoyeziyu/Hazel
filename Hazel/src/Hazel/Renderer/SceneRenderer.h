#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/glm.hpp>

namespace Hazel {

	class EditorCamera;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;

		void Init();

		void SetGridEnabled(bool enabled) { m_GridEnabled = enabled; }
		bool IsGridEnabled() const { return m_GridEnabled; }

		void BeginScene(const EditorCamera& camera);
		void BeginScene(const glm::mat4& viewProjection);
		void EndScene();

		void SubmitMesh(const glm::mat4& transform, const glm::vec4& color);
		void SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const glm::vec4& color);

		const Ref<VertexArray>& GetDefaultCubeMesh() const { return m_CubeVertexArray; }
		void RenderGrid();

		void Prepare2DOverlay();
		void RestoreAfter2D();

	private:
		Ref<Shader> m_MeshShader;
		Ref<Shader> m_GridShader;
		Ref<VertexArray> m_CubeVertexArray;
		Ref<VertexArray> m_GridVertexArray;

		glm::mat4 m_ViewProjection = glm::mat4(1.0f);
		bool m_GridEnabled = true;
	};

}
