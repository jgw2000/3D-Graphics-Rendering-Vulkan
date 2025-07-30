#include "BaseApp.h"
#include "IPipelineBuilder.h"

#include <array>

namespace jgw
{
    BaseApp::BaseApp(const WindowConfig& config)
    {
        windowPtr = std::make_unique<Window>(config);
        contextPtr = std::make_unique<VulkanContext>();
    }

    void BaseApp::Start()
    {
        if (!Initialize())
            return;

        GLFWwindow* handle = windowPtr->GetHandle();
        while (!glfwWindowShouldClose(handle))
        {
            glfwPollEvents();

            Update();
            
            if (!iconified && contextPtr->BeginRender())
            {
                Render(contextPtr->GetCommandBuffer());
                contextPtr->EndRender();
            }
        }

        contextPtr->WaitDeviceIdle();
        Cleanup();
    }

    bool BaseApp::Initialize()
    {
        if (!windowPtr->Initialize())
            return false;

        GLFWwindow* handle = windowPtr->GetHandle();
        SetCallback(handle);

        auto instanceLayers = GetInstanceLayers();
        auto instanceExtensions = GetInstanceExtensions();
        auto deviceExtensions = GetDeviceExtensions();

        if (!contextPtr->Initialize(handle, instanceLayers, instanceExtensions, deviceExtensions))
            return false;

        return true;
    }

    void BaseApp::Cleanup()
    {
        contextPtr.reset();
        windowPtr.reset();
    }

    void BaseApp::OnResize(int width, int height)
    {
        contextPtr->WindowResize();
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

    std::vector<const char*> BaseApp::GetDeviceExtensions() const
    {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
    }

    void BaseApp::TransitionImageLayout(
        vk::CommandBuffer       commandBuffer,
        vk::Image               image,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags         srcAccessMask,
        vk::AccessFlags         dstAccessMask,
        vk::PipelineStageFlags  srcStage,
        vk::PipelineStageFlags  dstStage
    )
    {
        vk::ImageMemoryBarrier imageMemoryBarrier{
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .image = contextPtr->GetSwapchain()->GetImage(),
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        commandBuffer.pipelineBarrier(srcStage, dstStage, {}, {}, {}, imageMemoryBarrier);
    }

    void BaseApp::SetCallback(GLFWwindow* handle)
    {
        glfwSetWindowUserPointer(handle, this);

        glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnKey(key, scancode, action, mods);

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        });

        glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
            if (width <= 0 || height <= 0)
                return;

            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnResize(width, height);
        });

        glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            app->iconified = iconified;
        });
    }
}
