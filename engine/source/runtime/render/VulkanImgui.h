#pragma once

#include "Macro.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <functional>

namespace jgw
{
    class VulkanImgui final
    {
    public:
        CLASS_COPY_MOVE_DELETE(VulkanImgui)

        VulkanImgui();
        ~VulkanImgui();

        bool Initialize(
            void*                 window,
            VkInstance            instance,
            VkPhysicalDevice      physicalDevice,
            VkDevice              device,
            VkQueue               queue,
            uint32_t              queueFamily,
            uint32_t              imageCount,
            VkFormat              colorFormat,
            VkFormat              depthFormat,
            uint32_t              apiVersion,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT
        );

        void BeginFrame();
        void EndFrame();
        void Render(VkCommandBuffer commandBuffer);

    private:
        VkDescriptorPool descriptorPool{};
    };
}
