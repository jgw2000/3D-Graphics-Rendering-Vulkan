#include "BaseApp.h"

namespace jgw
{
    BaseApp::BaseApp()
    {
        WindowConfig config = {};
        windowPtr = std::make_unique<Window>(config);
		contextPtr = std::make_unique<RenderContext>();
    }

    BaseApp::BaseApp(const WindowConfig& config)
    {
        windowPtr = std::make_unique<Window>(config);
    }

    void BaseApp::Run()
    {
        if (!Initialize())
            return;

        GLFWwindow* handle = windowPtr->GetHandle();
        while (!glfwWindowShouldClose(handle))
        {
            glfwPollEvents();
        }
    }

    bool BaseApp::Initialize()
    {
        if (!windowPtr->Initialize())
            return false;

        GLFWwindow* handle = windowPtr->GetHandle();
        glfwSetWindowUserPointer(handle, this);
        glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnKey(key, scancode, action, mods);

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        });

		auto instanceLayers = GetInstanceLayers();
		auto instanceExtensions = GetInstanceExtensions();

        if (!contextPtr->Initialize(instanceLayers, instanceExtensions))
            return false;

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
        return { };
	}
}
