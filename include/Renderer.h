#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"


#if defined(WIN32)


#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
		x;\
		ASSERT(GLLogCall(#x, __FILE__, __LINE__));

#endif
void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
	void Clear();
	void Draw(VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};
#endif