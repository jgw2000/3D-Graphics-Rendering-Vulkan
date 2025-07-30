#pragma once

#include "Macro.h"
#include "Window.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "IPipelineBuilder.h"

#include <VmaUsage.h>
#include <vector>
#include <memory>

namespace jgw
{
    class VulkanContext final
    {
    public:
        CLASS_COPY_MOVE_DELETE(VulkanContext)

        VulkanContext();
        ~VulkanContext();

        bool Initialize(
            GLFWwindow* handle,
            const std::vector<const char*>& requestInstanceLayers,
            const std::vector<const char*>& requestInstanceExtensions,
            const std::vector<const char*>& requestDeviceExtensions,
            uint32_t apiVersion = VK_API_VERSION_1_4
        );

        bool BeginRender();
        void EndRender();
        void WindowResize();
        void WaitDeviceIdle();
        void WaitQueueIdle();

        vk::CommandBuffer GetCommandBuffer() { return commandBuffers[currentFrame]; }
        VulkanSwapchain* GetSwapchain() { return swapchainPtr.get(); }

        vk::Pipeline CreateGraphicsPipeline(IPipelineBuilder& pd);

        std::unique_ptr<VulkanBuffer> CreateBuffer(
            vk::DeviceSize size,
            vk::BufferUsageFlags bufferUsage,
            vma::AllocationCreateFlags flags = {},
            vma::MemoryUsage memoryUsage = vma::MemoryUsage::eAuto
        );

        void UploadBuffer(const void* data, VulkanBuffer* srcBuffer, VulkanBuffer* dstBuffer);

    private:
        bool CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const;
        bool CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const;
        bool CheckDeviceExtensionSupport(const std::vector<const char*>& requestDeviceExtensions) const;

        uint32_t GetQueueFamilyIndex(vk::QueueFlags queueFlags) const;

    private:
        vk::Instance instance{};
        vk::SurfaceKHR surface{};
        vk::PhysicalDevice physicalDevice{};
        vk::Device device{};
        vk::Queue graphicsQueue{};
        vk::CommandPool commandPool{};
        std::unique_ptr<VulkanSwapchain> swapchainPtr;

        std::vector<vk::CommandBuffer> commandBuffers;
        std::vector<vk::Fence> fences;
        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Pipeline> pipelines;
        std::vector<vk::PipelineLayout> pipelineLayouts;

        vma::VulkanFunctions vulkanFuncs{};
        vma::Allocator vmaAllocator{};

        GLFWwindow* windowHandle = nullptr;
        uint32_t graphicsFamilyIndex = 0;
        uint32_t frameInFlight = 3;
        uint32_t currentFrame = 0;
        uint32_t apiVersion = VK_API_VERSION_1_4;
    };
}
