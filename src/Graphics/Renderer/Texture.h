#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include "Shader.h"

class Texture
{
public:
    Texture();
    Texture(const std::string filepath, const std::string filename, std::string texType);
    Texture(const std::string filepathname, std::string texType, bool isPgm);

    ~Texture();

    void texUnit(Shader &shader, const char *uniform, GLuint _unit);
    void Bind();
    void Bind(int width, int height, uint8_t *rgbData);
    void Unbind();
    void Delete();

    std::string &getType() {
        return m_type;
    }

    std::string &getPath() {
        return m_filepath;
    }

    std::string &getName() {
        return m_filename;
    }

    GLuint getID() {
        return m_TextureID;
    }

private:
    GLuint m_TextureID;
    std::string m_type;
    bool m_isPgm;
    std::string m_filepath;
    std::string m_filename;
    std::string m_filepathName;
    std::string m_tempname;

};

#endif