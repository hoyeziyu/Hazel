#include "hzpch.h"
#include "SceneRenderer.h"

#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/RenderCommand.h"

#include <glad/glad.h>

namespace Hazel {

	void SceneRenderer::Init()
	{
		m_MeshShader = Shader::Create("assets/shaders/Mesh.glsl");
		m_GridShader = Shader::Create("assets/shaders/Grid.glsl");

		float cubeVertices[] = {
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
		};

		uint32_t cubeIndices[] = {
			0, 1, 2, 2, 3, 0,
			1, 4, 7, 7, 2, 1,
			4, 5, 6, 6, 7, 4,
			5, 0, 3, 3, 6, 5,
			3, 2, 7, 7, 6, 3,
			5, 4, 1, 1, 0, 5
		};

		Ref<VertexBuffer> cubeVB = VertexBuffer::Create(cubeVertices, (uint32_t)sizeof(cubeVertices));
		cubeVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});

		Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, (uint32_t)(sizeof(cubeIndices) / sizeof(uint32_t)));

		m_CubeVertexArray = VertexArray::Create();
		m_CubeVertexArray->AddVertexBuffer(cubeVB);
		m_CubeVertexArray->SetIndexBuffer(cubeIB);

		const float gridExtent = 100.0f;
		float gridVertices[] = {
			-gridExtent, 0.0f, -gridExtent, -gridExtent, -gridExtent,
			 gridExtent, 0.0f, -gridExtent,  gridExtent, -gridExtent,
			 gridExtent, 0.0f,  gridExtent,  gridExtent,  gridExtent,
			-gridExtent, 0.0f,  gridExtent, -gridExtent,  gridExtent,
		};

		uint32_t gridIndices[] = { 0, 1, 2, 2, 3, 0 };

		Ref<VertexBuffer> gridVB = VertexBuffer::Create(gridVertices, (uint32_t)sizeof(gridVertices));
		gridVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});

		Ref<IndexBuffer> gridIB = IndexBuffer::Create(gridIndices, (uint32_t)(sizeof(gridIndices) / sizeof(uint32_t)));

		m_GridVertexArray = VertexArray::Create();
		m_GridVertexArray->AddVertexBuffer(gridVB);
		m_GridVertexArray->SetIndexBuffer(gridIB);
	}

	void SceneRenderer::BeginScene(const EditorCamera& camera)
	{
		BeginScene(camera.GetViewProjection());
	}

	void SceneRenderer::BeginScene(const glm::mat4& viewProjection)
	{
		m_ViewProjection = viewProjection;
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	void SceneRenderer::EndScene()
	{
	}

	void SceneRenderer::SubmitMesh(const glm::mat4& transform, const glm::vec4& color)
	{
		SubmitMesh(m_CubeVertexArray, transform, color);
	}

	void SceneRenderer::SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const glm::vec4& color)
	{
		if (!vertexArray)
			return;

		m_MeshShader->Bind();
		m_MeshShader->SetMat4("u_ViewProjection", m_ViewProjection);
		m_MeshShader->SetMat4("u_Transform", transform);
		m_MeshShader->SetFloat4("u_Color", color);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void SceneRenderer::RenderGrid()
	{
		if (!m_GridEnabled)
			return;

		// Grid is a translucent overlay; must not write depth or 2D sprites z-fight / stripe.
		glDepthMask(GL_FALSE);

		m_GridShader->Bind();
		m_GridShader->SetMat4("u_ViewProjection", m_ViewProjection);
		m_GridShader->SetMat4("u_Transform", glm::mat4(1.0f));
		m_GridShader->SetFloat("u_GridScale", 1.0f);
		m_GridShader->SetFloat("u_GridSize", 0.02f);

		m_GridVertexArray->Bind();
		RenderCommand::DrawIndexed(m_GridVertexArray);

		glDepthMask(GL_TRUE);
	}

	void SceneRenderer::Prepare2DOverlay()
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void SceneRenderer::RestoreAfter2D()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

}
