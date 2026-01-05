#include "VertexBuffer.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer(std::vector<Vertex> &vertices)
{
    glGenBuffers(1, &m_VetexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VetexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_VetexBufferID);
}

void VertexBuffer::BufferDataModification(std::vector<Vertex> &vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VetexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VetexBufferID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
