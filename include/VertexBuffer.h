#ifndef VERTEXBUFFER_H
#define VERTEXBUFFRT_H


class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();
	
	void Bind() const;
	void Unbind() const;
		
private:
	unsigned int m_RendererID;

};

#endif