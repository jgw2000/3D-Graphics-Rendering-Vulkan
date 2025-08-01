#pragma once

#include "Macro.h"

#include <VmaUsage.h>

namespace jgw
{
    struct TextureDesc
    {
        vk::ImageCreateFlags flags = {};
        vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eColorAttachment;
        vk::ImageType imageType = vk::ImageType::e2D;
        vk::ImageViewType viewType = vk::ImageViewType::e2D;
        vk::Format format = vk::Format::eR8G8B8A8Srgb;
        vk::Extent3D extent = {};
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
        std::vector<uint32_t> queueFamilyIndices = {};
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
    };

    struct VmaAllocationDesc
    {
        vma::AllocationCreateFlags flags = {};
        vma::MemoryUsage usage = vma::MemoryUsage::eAutoPreferDevice;
        float priority = 1.0f;
    };

    class VulkanTexture final
    {
        friend class VulkanContext;

    public:
        CLASS_COPY_MOVE_DELETE(VulkanTexture)

        explicit VulkanTexture(vk::Device device, vma::Allocator allocator, const TextureDesc& desc, const VmaAllocationDesc& allocDesc = {});

        ~VulkanTexture();

        void TransitionLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout layout);

        vk::Format GetFormat() const { return desc.format; }
        vk::ImageView GetView() const { return imageView; }

    private:
        vma::Allocator vmaAllocator;
        vma::Allocation vmaAllocation;
        vma::AllocationInfo vmaAllocationInfo;

        vk::Device device;
        vk::Image image;
        vk::ImageView imageView;
        
        TextureDesc desc;
    };
}
