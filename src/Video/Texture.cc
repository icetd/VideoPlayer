#include "Texuture.h"
#include <stdlib.h>
#include <stb_image.h>
#include "log.h"

Texture::Texture()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_TextureID);
}

Texture::Texture(const std::string filepathname, std::string texType, bool isPgm)
{
	//std::cout << "Texture Constructor" << std::endl;
	m_filepathName = filepathname;
    m_type = texType;
	m_isPgm = m_isPgm;

	glGenTextures(1, &m_TextureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(m_filepathName.c_str(), &width, &height, &nrComponents, 4);
	if (bytes) {
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		stbi_image_free(bytes);
	} else {
		stbi_image_free(bytes);
	}
}


Texture::~Texture()
{
}

void Texture::bind(int width, int height, uint8_t* rgbData)
{
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
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
