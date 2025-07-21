#include "BaseApp.h"

namespace jgw
{
    BaseApp::BaseApp(const WindowConfig& config)
    {
        contextPtr = std::make_unique<RenderContext>(config);
    }

    void BaseApp::Run()
    {
        if (!Initialize())
            return;

        contextPtr->MainLoop();
    }

    bool BaseApp::Initialize()
    {
        auto instanceLayers = GetInstanceLayers();
        auto instanceExtensions = GetInstanceExtensions();

        if (!contextPtr->Initialize(instanceLayers, instanceExtensions))
            return false;

		GLFWwindow* handle = contextPtr->GetWindowHandle();
        glfwSetWindowUserPointer(handle, this);
        glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnKey(key, scancode, action, mods);

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        });

        return true;
    }

    std::vector<const char*> BaseApp::GetInstanceLayers() const
    {
#if defined(_DEBUG)
        return { "VK_LAYER_KHRONOS_validation" };
#else
        return { };
#endif
    }

    std::vector<const char*> BaseApp::GetInstanceExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if defined(_DEBUG)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return extensions;
    }
}
