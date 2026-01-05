#ifndef RENDERER_H
#define RENDERER_H

#pragma once
#include <glad/glad.h>
#include <iostream>

#include "../Buffers/VertexArray.h"
#include "../Buffers/IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

class Renderer
{
public:
	Renderer(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture> textures);
	~Renderer();

	void Clear();
	void DrawLine(Shader& shader);
	void DrawLineStrip(Shader &shader, unsigned int size);
	void DrawLineStripPoint(Shader &shader, unsigned int size);
	void DrawTriangle(Shader &shader, GLenum mode);

private:
	std::vector<Vertex> m_vertices;
	std::vector<GLuint> m_indices;
	std::vector<Texture> m_textures;
	
    VertexArray m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
};

#endif