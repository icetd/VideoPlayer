#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VideoView.h"

class Application
{
public:
    Application(const char *appName, int w, int h);
    ~Application();

    void run();
    void wait();

private:
    std::string m_appName;
    GLFWwindow *m_window;

    void mainUI();

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    bool m_isDisplay;
    bool m_isMenuExit;
    double m_fps;

    std::unique_ptr<VideoView> m_VideoView;
    int m_width;
    int m_height;
};

#endif