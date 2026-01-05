#include "Scene.h"
#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static Scene *Instance = nullptr;

Scene::Scene()
{
    Instance = this;
}

Scene::~Scene()
{
}

Scene *Scene::GetInstance() { return Instance; }

void Scene::UpdateStatus(Shader &shader)
{
}