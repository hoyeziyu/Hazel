#include "hzpch.h"
#include "SceneRenderer.h"

#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel {

	namespace {

		Ref<VertexArray> CreateCubeVertexArray()
		{
			// 24 vertices (4 per face) with UVs for textured materials
			float cubeVertices[] = {
				// Front (+Z)
				-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
				 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
				// Back (-Z)
				 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
				 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
				// Top (+Y)
				-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
				 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
				// Bottom (-Y)
				-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
				 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f, 1.0f, 1.0f,
				-0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
				// Right (+X)
				 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
				 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
				// Left (-X)
				-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
				-0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
				-0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
			};

			uint32_t cubeIndices[] = {
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4,
				8, 9, 10, 10, 11, 8,
				12, 13, 14, 14, 15, 12,
				16, 17, 18, 18, 19, 16,
				20, 21, 22, 22, 23, 20
			};

			Ref<VertexBuffer> cubeVB = VertexBuffer::Create(cubeVertices, (uint32_t)sizeof(cubeVertices));
			cubeVB->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});

			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, (uint32_t)(sizeof(cubeIndices) / sizeof(uint32_t)));

			Ref<VertexArray> vertexArray = VertexArray::Create();
			vertexArray->AddVertexBuffer(cubeVB);
			vertexArray->SetIndexBuffer(cubeIB);
			return vertexArray;
		}

	}

	void SceneRenderer::Init()
	{
		m_MeshShader = Shader::Create("assets/shaders/Mesh.glsl");
		m_GridShader = Shader::Create("assets/shaders/Grid.glsl");

		m_WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whitePixel = 0xffffffff;
		m_WhiteTexture->SetData(&whitePixel, sizeof(uint32_t));

		m_CubeVertexArray = CreateCubeVertexArray();

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
		MeshMaterialData material;
		material.ColorTint = color;
		SubmitMesh(m_CubeVertexArray, transform, material);
	}

	void SceneRenderer::SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const glm::vec4& color)
	{
		MeshMaterialData material;
		material.ColorTint = color;
		SubmitMesh(vertexArray, transform, material);
	}

	void SceneRenderer::SubmitMesh(const Ref<VertexArray>& vertexArray, const glm::mat4& transform, const MeshMaterialData& material)
	{
		if (!vertexArray)
			return;

		m_MeshShader->Bind();
		m_MeshShader->SetMat4("u_ViewProjection", m_ViewProjection);
		m_MeshShader->SetMat4("u_Transform", transform);
		m_MeshShader->SetFloat4("u_Color", material.ColorTint);
		m_MeshShader->SetFloat3("u_AlbedoColor", material.AlbedoColor);

		const bool useAlbedoMap = material.AlbedoTexture != nullptr;
		m_MeshShader->SetInt("u_UseAlbedoMap", useAlbedoMap ? 1 : 0);
		if (useAlbedoMap)
			material.AlbedoTexture->Bind(0);
		else
			m_WhiteTexture->Bind(0);
		m_MeshShader->SetInt("u_AlbedoTexture", 0);

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
