#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Texture
{
public:
    Texture();
    Texture(const std::string filepathname, std::string texType, bool isPgm);
    ~Texture();

    void bind(int width, int height, uint8_t *rgbData);
    void unbind() const;
    inline int getId() const { return m_TextureID; }

private:
    GLuint m_TextureID;
    std::string m_type;
    bool m_isPgm;
    std::string m_filepathName;
};

#endif