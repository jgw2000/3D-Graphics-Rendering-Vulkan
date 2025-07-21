#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "RenderContext.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define FMT_UNICODE 0
#include <spdlog/spdlog.h>

namespace jgw
{
    RenderContext::RenderContext(const WindowConfig& config)
        : windowPtr(std::make_unique<Window>(config))
    {
	}

    RenderContext::~RenderContext()
    {
        if (instance)
        {
            instance.destroy();
            instance = nullptr;
        }
    }

    bool RenderContext::Initialize(
        const std::vector<const char*>& requestInstanceLayers,
        const std::vector<const char*>& requestInstanceExtensions,
        uint32_t apiVersion
    )
    {
        if (!windowPtr->Initialize())
        {
            spdlog::error("Failed to initialize window");
            return false;
		}

        try
        {
            // initialize minimal set of function pointers
            VULKAN_HPP_DEFAULT_DISPATCHER.init();

            if (!CheckInstanceLayerSupport(requestInstanceLayers) || !CheckInstanceExtensionSupport(requestInstanceExtensions))
            {
                return false;
            }

            vk::ApplicationInfo appInfo{
                .pApplicationName = "Vulkan Project",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "Vulkan",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = apiVersion
            };

            vk::InstanceCreateInfo instanceInfo{
                .pApplicationInfo = &appInfo,
            };

            instance = vk::createInstance(instanceInfo);

            // initialize function pointers for instance
            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
        }
        catch (const vk::SystemError& err)
        {
            spdlog::error("vk::SystemError: {}", err.what());
            return false;
        }
        

        return true;
    }

    void RenderContext::MainLoop()
    {
        GLFWwindow* handle = windowPtr->GetHandle();
        while (!glfwWindowShouldClose(handle))
        {
            glfwPollEvents();
        }
    }

    bool RenderContext::CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const
    {
        if (requestInstanceLayers.empty())
            return true;

        auto availableLayers = vk::enumerateInstanceLayerProperties();
        for (const auto& layer : requestInstanceLayers)
        {
            bool found = false;
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(layer, availableLayer.layerName.data()) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                spdlog::error("Instance layer {} not found", layer);
                return false;
            }
        }

        return true;
    }

    bool RenderContext::CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const
    {
        if (requestInstanceExtensions.empty())
            return true;

        auto availableExtensions = vk::enumerateInstanceExtensionProperties();
        for (const auto& extension : requestInstanceExtensions)
        {
            bool found = false;
            for (const auto& availableExtension : availableExtensions)
            {
                if (strcmp(extension, availableExtension.extensionName.data()) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                spdlog::error("Instance extension {} not found", extension);
                return false;
            }
        }
        return true;
    }
}
