#include "Renderer.h"
#include "../../Core/log.h"
#include <glm/gtx/string_cast.hpp>

Renderer::Renderer(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture> textures)
	: m_vertices(vertices)
	, m_indices(indices)
	, m_textures(textures)
{
	m_vao.Bind();
	m_vbo = new VertexBuffer(m_vertices);
	m_ibo = new IndexBuffer(m_indices);

	m_vao.AddBuffer(*m_vbo, 0, 3, sizeof(Vertex), (void*)0);
	m_vao.AddBuffer(*m_vbo, 1, 3, sizeof(Vertex), (void*)offsetof(Vertex, color));
	m_vao.AddBuffer(*m_vbo, 2, 3, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	m_vao.AddBuffer(*m_vbo, 3, 2, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	m_vao.AddBuffer(*m_vbo, 4, 3, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	m_vao.AddBuffer(*m_vbo, 5, 3, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
	m_vao.AddBuffer(*m_vbo, 5, 4, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	m_vao.AddBuffer(*m_vbo, 6, 4, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

	m_vao.Unbind();
	m_vbo->Unbind();
	m_ibo->Unbind();

	// LOG(INFO, "Renderer Constructed");
}

Renderer::~Renderer()
{
	delete m_vbo;
	delete m_ibo;
}

void Renderer::DrawLine(Shader& shader)
{
	m_vao.Bind();
	glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_INT, 0);
	m_vao.Unbind();
    glActiveTexture(GL_TEXTURE0);
}

void Renderer::DrawLineStrip(Shader &shader, unsigned int size)
{
	m_vao.Bind();
    glDrawArrays(GL_LINE_STRIP, 0, size);
	m_vao.Unbind();
    glActiveTexture(GL_TEXTURE0);
}

void Renderer::DrawLineStripPoint(Shader &shader, unsigned int size)
{
	m_vao.Bind();
    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, size);
	m_vao.Unbind();
	glActiveTexture(GL_TEXTURE0);
}

void Renderer::DrawTriangle(Shader& shader, GLenum mode)
{
    m_vao.Bind();
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	
	for (unsigned int i = 0; i < m_textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number;
		std::string name = m_textures[i].getType();
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); 
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); 
		else if (name == "texture_height")
			number = std::to_string(heightNr++);

		m_textures[i].Bind();
		m_textures[i].texUnit(shader, (name + number).c_str(), i);
	}

	glPolygonMode(GL_FRONT_AND_BACK, mode);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	m_vao.Unbind();

	// always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Renderer::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}