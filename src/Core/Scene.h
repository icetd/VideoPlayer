#ifndef SENCEROBOT_H
#define SENCEROBOT_H

#include "../Graphics/Renderer/Shader.h"
#include "../Graphics/Renderer/Texture.h"
#include "../Graphics/Renderer/Camera.h"
#include "../Graphics/Renderer/Renderer.h"

class Scene
{
public:
    Scene();
    ~Scene();

    static Scene *GetInstance();

    void UpdateStatus(Shader &shader);

private:
    Renderer *m_Render;
};

#endif