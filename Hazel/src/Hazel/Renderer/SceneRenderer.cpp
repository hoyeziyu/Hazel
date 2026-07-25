#include "hzpch.h"
#include "SceneRenderer.h"

#include "Hazel/Asset/MeshSource.h"
#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel {

	namespace {

		Ref<VertexArray> CreateCubeVertexArray()
		{
			std::vector<glm::vec3> positions = {
				{ -0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f }, { 0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f },
				{  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f },
				{ -0.5f,  0.5f,  0.5f }, { 0.5f,  0.5f,  0.5f }, { 0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
				{ -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f },
				{  0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f },
				{ -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f },
			};

			std::vector<glm::vec2> texCoords(positions.size());
			for (size_t face = 0; face < 6; face++)
			{
				texCoords[face * 4 + 0] = { 0.0f, 0.0f };
				texCoords[face * 4 + 1] = { 1.0f, 0.0f };
				texCoords[face * 4 + 2] = { 1.0f, 1.0f };
				texCoords[face * 4 + 3] = { 0.0f, 1.0f };
			}

			std::vector<uint32_t> indices = {
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4,
				8, 9, 10, 10, 11, 8,
				12, 13, 14, 14, 15, 12,
				16, 17, 18, 18, 19, 16,
				20, 21, 22, 22, 23, 20
			};

			std::vector<glm::vec3> normals = MeshSource::GenerateSmoothNormals(positions, indices);

			std::vector<float> flatVertices;
			flatVertices.reserve(positions.size() * 8);
			for (size_t i = 0; i < positions.size(); i++)
			{
				flatVertices.push_back(positions[i].x);
				flatVertices.push_back(positions[i].y);
				flatVertices.push_back(positions[i].z);
				flatVertices.push_back(normals[i].x);
				flatVertices.push_back(normals[i].y);
				flatVertices.push_back(normals[i].z);
				flatVertices.push_back(texCoords[i].x);
				flatVertices.push_back(texCoords[i].y);
			}

			Ref<VertexBuffer> cubeVB = VertexBuffer::Create(flatVertices.data(), (uint32_t)(flatVertices.size() * sizeof(float)));
			cubeVB->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});

			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(indices.data(), (uint32_t)indices.size());

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

	void SceneRenderer::BeginScene(const EditorCamera& camera, const SceneEnvironmentData& environment)
	{
		BeginScene(camera.GetViewProjection(), environment);
	}

	void SceneRenderer::BeginScene(const glm::mat4& viewProjection, const SceneEnvironmentData& environment)
	{
		m_ViewProjection = viewProjection;
		m_Environment = environment;
		m_Stats.Reset();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	void SceneRenderer::EndScene()
	{
	}

	void SceneRenderer::ApplyEnvironmentUniforms()
	{
		m_MeshShader->SetFloat3("u_CameraPosition", m_Environment.CameraPosition);
		m_MeshShader->SetFloat3("u_LightDirection", m_Environment.LightDirection);
		m_MeshShader->SetFloat3("u_LightRadiance", m_Environment.LightRadiance);
		m_MeshShader->SetFloat("u_LightIntensity", m_Environment.LightIntensity);
		m_MeshShader->SetInt("u_HasDirectionalLight", m_Environment.HasDirectionalLight ? 1 : 0);
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
		ApplyEnvironmentUniforms();

		const bool useAlbedoMap = material.AlbedoTexture != nullptr;
		m_MeshShader->SetInt("u_UseAlbedoMap", useAlbedoMap ? 1 : 0);
		if (useAlbedoMap)
			material.AlbedoTexture->Bind(0);
		else
			m_WhiteTexture->Bind(0);
		m_MeshShader->SetInt("u_AlbedoTexture", 0);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);

		m_Stats.DrawCalls++;
		if (vertexArray->GetIndexBuffer())
			m_Stats.TriangleCount += vertexArray->GetIndexBuffer()->GetCount() / 3;
	}

	void SceneRenderer::RenderGrid()
	{
		if (!m_GridEnabled)
			return;

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
