#include "Window.h"

namespace jgw
{
    Window::Window(const WindowConfig& config) :
        windowConfig(config)
    {
    }

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Initialize()
    {
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        switch (windowConfig.mode)
        {
        case EWindowMode::Windowed:
            handle = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title.c_str(), nullptr, nullptr);
            break;

        case EWindowMode::Fullscreen:
            handle = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title.c_str(), glfwGetPrimaryMonitor(), nullptr);
            break;

        case EWindowMode::WindowedFullscreen:
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

            handle = glfwCreateWindow(mode->width, mode->height, windowConfig.title.c_str(), monitor, nullptr);
            windowConfig.width = mode->width;
            windowConfig.height = mode->height;
            break;

        default:
            break;
        }

        if (handle == nullptr)
            return false;

        glfwGetFramebufferSize(handle, &windowConfig.width, &windowConfig.height);
    
        return true;
    }

    void Window::Destroy()
    {
        glfwDestroyWindow(handle);
        glfwTerminate();
    }
}
