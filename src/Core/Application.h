#ifndef APPLICATION_H
#define APPLICATION_H

#include "Layer.h"
#include "../Graphics/Renderer/Camera.h"
#include "../Graphics/Buffers/FrameBuffer.h"
#include "../Utils/INIReader.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application 
{
public:
    Application(const char *appName, int w, int h);
    ~Application();

    static Application *GetInstance();
    void Run();

    template <typename T>
    void PushLayer()
    {
        static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
        m_LayerStack.emplace_back(std::make_shared<T>());
        m_LayerStack.back()->OnAttach();
    }

    void PushLayer(const std::shared_ptr<Layer> &layer);
    void Close();

    void InitCamera(Camera* camera, FrameBuffer* frameBuffer) {
	    m_camera = camera;
	    m_frameBuffer = frameBuffer;
    }

    float GetTime();

    static void window_size_callback_static(GLFWwindow *window, int width, int height);
    static void mouse_callback_static(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback_static(GLFWwindow *window, double xpos, double ypos);

    void window_size_callback(GLFWwindow *window, int width, int height);
    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    const int GetWidth() {
        return m_Width;
    }

    const int GetHeight() {
        return m_Height;
    }
private:
    std::string m_AppName;
    int m_Width;
    int m_Height;
    GLFWwindow *m_WindowHandler;

    float m_TimeStep = 0.0f;
    float m_FrameTime = 0.0f;
    float m_LastFrameTime = 0.0f;
    std::vector<std::shared_ptr<Layer>> m_LayerStack;

    glm::vec2 lastMousePosRightClick = glm::vec2(0.0f, 0.0f);
    glm::vec2 currentMousePosClick = glm::vec2(0.0f, 0.0f);
    bool firstMouseClick;

    Camera *m_camera;
    FrameBuffer *m_frameBuffer;

    void Init();
    void Shutdown();
    INIReader *m_config;
};

#endif