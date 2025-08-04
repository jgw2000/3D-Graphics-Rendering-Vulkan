#pragma once

#include "Common.h"

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

        bool Initialize();
        void Destroy();

    private:
        GLFWwindow* handle = nullptr;
        WindowConfig windowConfig = {};
    };
}
