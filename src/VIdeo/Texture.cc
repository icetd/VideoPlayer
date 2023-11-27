#include "Texuture.h"
#include <stdlib.h>

Texture::Texture()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

Texture::~Texture()
{
}

void Texture::bind(int width, int height, uint8_t* rgbData)
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbData);
	glGenerateMipmap(GL_TEXTURE_2D);

}

void Texture::unbind() const
{
	glDeleteTextures(GL_TEXTURE_2D, 0);
}
