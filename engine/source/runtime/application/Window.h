#pragma once

#include "Macro.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include <string>

namespace jgw
{
    enum class EWindowMode
    {
        Windowed,
        Fullscreen,
        WindowedFullscreen
    };

    struct WindowConfig
    {
        std::string title = "";
        int width = 1280;
        int height = 800;
        EWindowMode mode = EWindowMode::Windowed;
    };

    class Window final
    {
    public:
        CLASS_COPY_MOVE_DELETE(Window)

        Window(const WindowConfig& config);
        ~Window();

        GLFWwindow* GetHandle() const { return handle; }
        uint32_t Width() const { return windowConfig.width; }
        uint32_t Height() const { return windowConfig.height; }

        bool Initialize();
        void Destroy();

    private:
        GLFWwindow* handle = nullptr;
        WindowConfig windowConfig = {};
    };
}
