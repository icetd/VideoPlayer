#include "../../Graphics/renderer/Texture.h"
#include "../../Core/log.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

Texture::Texture()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_TextureID);
	LOG(INFO, "Texture ID[%d] for Video", m_TextureID);
}

Texture::Texture(std::string filepath, std::string filename, std::string texType)
{
	m_filepath = filepath;
	m_filename = filename;
	m_tempname = m_filepath + '/' + m_filename;
	m_type = texType;

	glGenTextures(1, &m_TextureID);
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(m_tempname.c_str(), &width, &height, &nrComponents, 0);
	
	if (bytes) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, m_TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		LOG(INFO, "Texture ID[%d] Type[%s] Width[%d] Height[%d] Name[%s]", m_TextureID, m_type.c_str(), width, height, filename.c_str());
		stbi_image_free(bytes);
	} else {
		LOG(ERRO, "Texture failed to load at path : %s", m_tempname.c_str());
		stbi_image_free(bytes);
	}
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

void Texture::texUnit(Shader& shader, const char* uniform, GLuint _unit)
{
	shader.SetUniform1i(uniform, _unit);
}

void Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}


void Texture::Bind(int width, int height, uint8_t* rgbData)
{
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
    if (width != m_width || height != m_height) {
        // 初始化纹理，首次创建时使用 glTexImage2D
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbData);
        m_width = width;
        m_height = height;

        // 纹理参数设置
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 生成 Mipmap（如果需要）
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // 否则仅更新纹理数据
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbData);
    }
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &m_TextureID);
}

