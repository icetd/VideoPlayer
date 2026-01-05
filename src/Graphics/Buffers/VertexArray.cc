#include "VertexArray.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_VertexArray_ID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_VertexArray_ID);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, GLuint layout, GLuint numComponents, GLsizeiptr stride, void* offset)
{
    vb.Bind();
    glVertexAttribPointer(layout, numComponents, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    vb.Unbind();
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_VertexArray_ID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}