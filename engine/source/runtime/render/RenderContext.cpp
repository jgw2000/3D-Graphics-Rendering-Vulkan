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
        if (device)
        {
            device.destroy();
        }

        if (instance)
        {
            instance.destroySurfaceKHR(surface);
            instance.destroy();
            instance = nullptr;
        }
    }

    bool RenderContext::InitWindow()
    {
        if (!windowPtr->Initialize())
        {
            spdlog::error("Failed to initialize window");
            return false;
        }

        return true;
    }

    bool RenderContext::InitVulkan(
        const std::vector<const char*>& requestInstanceLayers,
        const std::vector<const char*>& requestInstanceExtensions,
        uint32_t apiVersion
    )
    {
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

            vk::InstanceCreateInfo instanceCI{
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = static_cast<uint32_t>(requestInstanceLayers.size()),
                .ppEnabledLayerNames = requestInstanceLayers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(requestInstanceExtensions.size()),
                .ppEnabledExtensionNames = requestInstanceExtensions.data()
            };

            instance = vk::createInstance(instanceCI);

            // initialize function pointers for instance
            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

            VkSurfaceKHR sur;
            auto result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), windowPtr->GetHandle(), nullptr, &sur);
            if (result != VK_SUCCESS)
            {
                spdlog::error("Failed to create window surface: {}", vk::to_string(static_cast<vk::Result>(result)));
                return false;
            }
            surface = vk::SurfaceKHR(sur);

            auto physicalDevices = instance.enumeratePhysicalDevices();
            for (const auto& pd : physicalDevices)
            {
                auto properties = pd.getProperties();
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                {
                    physicalDevice = pd;
                    break;
                }
            }

            if (physicalDevice == nullptr)
            {
                spdlog::error("No discrete gpu found");
                return false;
            }

            spdlog::info("Using physical device: {}", physicalDevice.getProperties().deviceName.data());

            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

            // Graphics queue
            vk::DeviceQueueCreateInfo deviceQueueCI{
                .queueFamilyIndex = GetQueueFamilyIndex(vk::QueueFlagBits::eGraphics),
                .queueCount = 1,
                .pQueuePriorities = new float[1] { 1.0f } // Default priority
            };
            queueCreateInfos.push_back(deviceQueueCI);

            // Compute queue
            deviceQueueCI.queueFamilyIndex = GetQueueFamilyIndex(vk::QueueFlagBits::eCompute);
            queueCreateInfos.push_back(deviceQueueCI);

            // Transfer queue
            deviceQueueCI.queueFamilyIndex = GetQueueFamilyIndex(vk::QueueFlagBits::eTransfer);
            queueCreateInfos.push_back(deviceQueueCI);


            vk::DeviceCreateInfo deviceCI{
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = static_cast<uint32_t>(requestInstanceLayers.size()),
                .ppEnabledLayerNames = requestInstanceLayers.data(),
                .enabledExtensionCount = 0, // No device extensions requested
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = nullptr // No specific features requested
            };

            device = physicalDevice.createDevice(deviceCI);

            // initialize function pointers for device
            VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

            graphicsQueue = device.getQueue(queueCreateInfos[0].queueFamilyIndex, 0);
            computeQueue = device.getQueue(queueCreateInfos[1].queueFamilyIndex, 0);
            transferQueue = device.getQueue(queueCreateInfos[2].queueFamilyIndex, 0);
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

    uint32_t RenderContext::GetQueueFamilyIndex(vk::QueueFlags queueFlags) const
    {
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        
        // Dedicated queue for graphics
        if ((queueFlags & vk::QueueFlagBits::eGraphics) == queueFlags)
        {
            for (uint32_t i = 0; i < queueFamilies.size(); ++i)
            {
                if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) && !(queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute))
                {
                    return i;
                }
            }
        }

        // Dedicated queue for compute
        if ((queueFlags & vk::QueueFlagBits::eCompute) == queueFlags)
        {
            for (uint32_t i = 0; i < queueFamilies.size(); ++i)
            {
                if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) && !(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics))
                {
                    return i;
                }
            }
        }

        // Dedicated queue for transfer
        if ((queueFlags & vk::QueueFlagBits::eTransfer) == queueFlags)
        {
            for (uint32_t i = 0; i < queueFamilies.size(); ++i)
            {
                if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) && !(queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute))
                {
                    return i;
                }
            }
        }

        for (uint32_t i = 0; i < queueFamilies.size(); ++i)
        {
            if ((queueFamilies[i].queueFlags & queueFlags) == queueFlags)
            {
                return i;
            }
        }

        throw std::runtime_error("Could not find a matching queue family index");
    }
}
