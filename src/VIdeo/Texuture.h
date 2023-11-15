#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture
{
public:
	Texture();
	~Texture();

    void bind(int width, int height, uint8_t *rgbData);
    void unbind() const;
    inline int getId() const { return m_ID; }

private:
	unsigned int m_ID;
};

#endif