#pragma once

#include "Macro.h"
#include "Window.h"

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

namespace jgw
{
    class RenderContext final
    {
    public:
        CLASS_COPY_MOVE_DELETE(RenderContext)

        RenderContext(const WindowConfig& config = {});
        ~RenderContext();

        bool InitWindow();

        bool InitVulkan(
            const std::vector<const char*>& requestInstanceLayers,
            const std::vector<const char*>& requestInstanceExtensions,
            uint32_t apiVersion = VK_API_VERSION_1_3
        );

        void MainLoop();

        GLFWwindow* GetWindowHandle() const
        {
            return windowPtr->GetHandle();
        }

    private:
        bool CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const;
        bool CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const;

        uint32_t GetQueueFamilyIndex(vk::QueueFlags queueFlags) const;

    private:
        std::unique_ptr<Window> windowPtr;

        vk::Instance instance{};
        vk::SurfaceKHR surface{};
        vk::PhysicalDevice physicalDevice{};
        vk::Device device{};
        vk::Queue graphicsQueue{};
        vk::Queue computeQueue{};
        vk::Queue transferQueue{};
    };
}
