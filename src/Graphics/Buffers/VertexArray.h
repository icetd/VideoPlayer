#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "VertexBuffer.h"
#include <glad/glad.h>

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(const VertexBuffer& vb, GLuint layout, GLuint numComponents, GLsizeiptr stride, void *offset);
    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_VertexArray_ID;
};

#endif