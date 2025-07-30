#include "VulkanContext.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <spdlog/spdlog.h>

namespace jgw
{
    VulkanContext::VulkanContext()
    {
    }

    VulkanContext::~VulkanContext()
    {
        vmaAllocator.destroy();

        if (device)
        {
            for (auto& fence : fences) device.destroy(fence);
            for (auto& semaphore : imageAvailableSemaphores) device.destroy(semaphore);
            for (auto& semaphore : renderFinishedSemaphores) device.destroy(semaphore);
            for (auto& layout : pipelineLayouts) device.destroyPipelineLayout(layout);
            for (auto& pipeline : pipelines) device.destroyPipeline(pipeline);

            swapchainPtr->Destroy();

            device.destroyCommandPool(commandPool);
            device.destroy();
        }

        if (instance)
        {
            instance.destroySurfaceKHR(surface);
            instance.destroy();
            instance = nullptr;
        }

        commandBuffers.clear();
        fences.clear();
        imageAvailableSemaphores.clear();
        renderFinishedSemaphores.clear();
    }

    bool VulkanContext::Initialize(
        GLFWwindow* handle,
        const std::vector<const char*>& requestInstanceLayers,
        const std::vector<const char*>& requestInstanceExtensions,
        const std::vector<const char*>& requestDeviceExtensions,
        uint32_t apiVersion
    )
    {
        this->apiVersion = apiVersion;

        try
        {
            // Create vulkan instance
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

            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

            // Create surface
            VkSurfaceKHR sur;
            auto result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), handle, nullptr, &sur);
            if (result != VK_SUCCESS)
            {
                spdlog::error("Failed to create window surface: {}", vk::to_string(static_cast<vk::Result>(result)));
                return false;
            }
            surface = vk::SurfaceKHR(sur);

            // Create physical device
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

            if (!CheckDeviceExtensionSupport(requestDeviceExtensions))
                return false;

            // Create logical device
            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

            graphicsFamilyIndex = GetQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
            vk::DeviceQueueCreateInfo deviceQueueCI{
                .queueFamilyIndex = graphicsFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = new float[1] { 1.0f } // Default priority
            };
            queueCreateInfos.push_back(deviceQueueCI);

            vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{
                .dynamicRendering = vk::True
            };

            vk::DeviceCreateInfo deviceCI{
                .pNext = &dynamicRenderingFeature,
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = static_cast<uint32_t>(requestInstanceLayers.size()),
                .ppEnabledLayerNames = requestInstanceLayers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(requestDeviceExtensions.size()),
                .ppEnabledExtensionNames = requestDeviceExtensions.data(),
                .pEnabledFeatures = nullptr // No specific features requested
            };

            device = physicalDevice.createDevice(deviceCI);

            VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

            // Create command pool and command buffer
            graphicsQueue = device.getQueue(graphicsFamilyIndex, 0);

            vk::CommandPoolCreateInfo commandPoolCI{
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = graphicsFamilyIndex
            };

            commandPool = device.createCommandPool(commandPoolCI);

            vk::CommandBufferAllocateInfo commandBufferAI{
                .commandPool = commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = frameInFlight
            };

            commandBuffers = device.allocateCommandBuffers(commandBufferAI);

            // Create swapchain
            swapchainPtr = std::make_unique<VulkanSwapchain>(physicalDevice, device, surface);
            swapchainPtr->Create(handle);

            // Create synchronization objects
            for (uint32_t i = 0; i < frameInFlight; ++i)
            {
                vk::FenceCreateInfo fenceCI{
                    .flags = vk::FenceCreateFlagBits::eSignaled // Start with signaled state
                };
                fences.push_back(device.createFence(fenceCI));

                vk::SemaphoreCreateInfo semaphoreCI{};
                imageAvailableSemaphores.push_back(device.createSemaphore(semaphoreCI));
                renderFinishedSemaphores.push_back(device.createSemaphore(semaphoreCI));
            }

            // Initialize VMA
            vulkanFuncs = vma::functionsFromDispatcher(VULKAN_HPP_DEFAULT_DISPATCHER);
            vma::AllocatorCreateInfo allocatorCI{
                .physicalDevice = physicalDevice,
                .device = device,
                .pVulkanFunctions = &vulkanFuncs,
                .instance = instance,
                .vulkanApiVersion = apiVersion
            };

            vmaAllocator = vma::createAllocator(allocatorCI);
        }
        catch (const vk::SystemError& err)
        {
            spdlog::error("vk::SystemError: {}", err.what());
            return false;
        }   

        windowHandle = handle;
        return true;
    }

    bool VulkanContext::BeginRender()
    {
        auto extent = swapchainPtr->GetExtent();
        if (extent.width <= 0 || extent.height <= 0)
        {
            return false;
        }

        // Wait for the current frame to finish
        auto result = device.waitForFences(fences[currentFrame], vk::True, UINT64_MAX);
        if (result != vk::Result::eSuccess)
        {
            spdlog::warn("Failed to wait for fence: {}", vk::to_string(result));
            return false;
        }

        device.resetFences(fences[currentFrame]);

        // Acquire the next image from the swapchain
        result = swapchainPtr->AcquireImage(imageAvailableSemaphores[currentFrame]);
        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            WindowResize();
            return false;
        }
        else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            spdlog::error("Failed to acquire swapchain image: {}", vk::to_string(result));
            return false;
        }

        // Begin command buffer recording
        vk::CommandBufferBeginInfo beginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        commandBuffers[currentFrame].begin(beginInfo);

        return true;
    }

    void VulkanContext::EndRender()
    {
        // End command buffer recording
        commandBuffers[currentFrame].end();

        // Submit the command buffer
        vk::SubmitInfo submitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
            .pWaitDstStageMask = new vk::PipelineStageFlags[1]{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[currentFrame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderFinishedSemaphores[currentFrame]
        };
        graphicsQueue.submit(submitInfo, fences[currentFrame]);

        // Present the swapchain image
        vk::PresentInfoKHR presentInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
            .swapchainCount = 1,
            .pSwapchains = swapchainPtr->GetHandle(),
            .pImageIndices = new uint32_t[1]{ swapchainPtr->GetImageIndex() }
        };
        auto result = graphicsQueue.presentKHR(presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            WindowResize();
        }
        else if (result != vk::Result::eSuccess)
        {
            spdlog::error("Failed to present swapchain image: {}", vk::to_string(result));
        }

        currentFrame = (currentFrame + 1) % frameInFlight;
    }

    void VulkanContext::WindowResize()
    {
        device.waitIdle();
        swapchainPtr->Create(windowHandle);
    }

    void VulkanContext::WaitDeviceIdle()
    {
        if (device)
            device.waitIdle();
    }

    void VulkanContext::WaitQueueIdle()
    {
        if (graphicsQueue)
            graphicsQueue.waitIdle();
    }

    vk::Pipeline VulkanContext::CreateGraphicsPipeline(IPipelineBuilder& pd)
    {
        auto shaderStages = pd.BuildShaderStages(device);
        auto vertexInputState = pd.BuildVertexInputState();
        auto inputAssemblyState = pd.BuildInputAssemblyState();
        auto viewportState = pd.BuildViewportState();
        auto rasterizationState = pd.BuildRasterizationState();
        auto multisampleState = pd.BuildMultisampleState();
        auto depthStencilState = pd.BuildDepthStencilState();
        auto colorBlendState = pd.BuildColorBlendState();
        auto dynamicState = pd.BuildDynamicState();
        auto pipelineLayout = pd.BuildLayout(device);
        pipelineLayouts.push_back(pipelineLayout);

        vk::PipelineRenderingCreateInfo renderingCI{
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = new vk::Format[1]{ swapchainPtr->GetFormat() },
        };

        vk::GraphicsPipelineCreateInfo pipelineCI{
            .pNext = &renderingCI,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout,
        };

        auto ret = device.createGraphicsPipeline(nullptr, pipelineCI);

        for (auto& shaderStage : shaderStages)
        {
            device.destroyShaderModule(shaderStage.module);
        }

        if (ret.result != vk::Result::eSuccess)
        {
            spdlog::error("Failed to create graphics pipeline: {}", vk::to_string(ret.result));
            return nullptr;
        }

        pipelines.push_back(ret.value);
        return ret.value;
    }

    std::unique_ptr<VulkanBuffer> VulkanContext::CreateBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags bufferUsage,
        vma::AllocationCreateFlags flags,
        vma::MemoryUsage memoryUsage
    )
    {
        return std::make_unique<VulkanBuffer>(size, bufferUsage, vmaAllocator, flags, memoryUsage);
    }

    void VulkanContext::UploadBuffer(const void* data, VulkanBuffer* srcBuffer, VulkanBuffer* dstBuffer)
    {
        memcpy(srcBuffer->mappedMemory, data, srcBuffer->size);

        auto commandBuffer = GetCommandBuffer();

        vk::CommandBufferBeginInfo beginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        commandBuffer.begin(beginInfo);

        vk::BufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = srcBuffer->size
        };
        commandBuffer.copyBuffer(srcBuffer->buffer, dstBuffer->buffer, copyRegion);

        commandBuffer.end();

        // Submit
        vk::SubmitInfo submitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
        };
        graphicsQueue.submit(submitInfo, nullptr);
    }

    bool VulkanContext::CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const
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

    bool VulkanContext::CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const
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

    bool VulkanContext::CheckDeviceExtensionSupport(const std::vector<const char*>& requestDeviceExtensions) const
    {
        if (requestDeviceExtensions.empty())
            return true;

        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        for (const auto& extension : requestDeviceExtensions)
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
                spdlog::error("Device extension {} not found", extension);
                return false;
            }
        }
        return true;
    }

    uint32_t VulkanContext::GetQueueFamilyIndex(vk::QueueFlags queueFlags) const
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
