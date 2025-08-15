#pragma once

#include "Common.h"
#include "Window.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "PipelineBuilder.h"

#include <ktx.h>
#include <ktxvulkan.h>

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
        void BeginCommand();
        void EndCommand();

        void WindowResize();
        void WaitDeviceIdle();
        void WaitQueueIdle();

        uint32_t GetApiVersion() const { return apiVersion; }
        uint32_t GetQueuFamily() const { return graphicsFamilyIndex; }
        vk::Instance GetInstance() const { return instance; }
        vk::PhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
        vk::Device GetDevice() const { return device; }
        vk::Queue GetQueue() const { return graphicsQueue; }
        vk::CommandBuffer GetCommandBuffer() const { return commandBuffers[currentFrame]; }
        VulkanSwapchain* GetSwapchain() const { return swapchainPtr.get(); }
        VulkanTexture* GetDepthTexture() const { return depthBuffer.get(); }

        std::unique_ptr<VulkanPipeline> CreateGraphicsPipeline(PipelineBuilder& pd);

        std::unique_ptr<VulkanBuffer> CreateBuffer(
            vk::DeviceSize size,
            vk::BufferUsageFlags bufferUsage,
            vma::AllocationCreateFlags flags = {},
            vma::MemoryUsage memoryUsage = vma::MemoryUsage::eAuto
        );

        std::unique_ptr<VulkanTexture> CreateTexture(const TextureDesc& desc, const VmaAllocationDesc& allocDesc = {});
        std::unique_ptr<VulkanTexture> CreateDepthTexture(vk::Format depthFormat = vk::Format::eD32Sfloat);

        void UploadBuffer(const void* data, VulkanBuffer* srcBuffer, VulkanBuffer* dstBuffer);
        void UploadTexture(const void* data, VulkanBuffer* srcBuffer, VulkanTexture* dstTexture);
        void UploadCubeTexture(ktxTexture* data, VulkanBuffer* srcBuffer, VulkanTexture* dstTexture);

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
        std::unique_ptr<VulkanTexture> depthBuffer;

        std::vector<vk::CommandBuffer> commandBuffers;
        std::vector<vk::Fence> fences;
        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;

        vma::VulkanFunctions vulkanFuncs{};
        vma::Allocator vmaAllocator{};

        GLFWwindow* windowHandle = nullptr;
        uint32_t graphicsFamilyIndex = 0;
        uint32_t frameInFlight = 3;
        uint32_t currentFrame = 0;
        uint32_t apiVersion = VK_API_VERSION_1_4;
    };
}
