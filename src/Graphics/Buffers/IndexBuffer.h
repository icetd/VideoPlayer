#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <glad/glad.h>
#include <vector>

class IndexBuffer
{
public:
	IndexBuffer(std::vector<GLuint>& indices);
	~IndexBuffer();
	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_IndexBufferID;
};

#endif