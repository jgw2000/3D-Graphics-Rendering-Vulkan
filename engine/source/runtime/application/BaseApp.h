#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

namespace jgw
{
    class BaseApp
    {
    public:
        void Run();
    };
}
