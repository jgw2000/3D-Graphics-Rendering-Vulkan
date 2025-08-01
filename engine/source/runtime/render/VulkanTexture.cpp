#include "VulkanTexture.h"

namespace jgw
{
    VulkanTexture::VulkanTexture(vk::Device device, vma::Allocator allocator, const TextureDesc& desc, const VmaAllocationDesc& allocDesc)
        : device(device)
        , vmaAllocator(allocator)
        , desc(desc)
    {
        vk::ImageCreateInfo imageCI{
            .flags = desc.flags,
            .imageType = desc.imageType,
            .format = desc.format,
            .extent = desc.extent,
            .mipLevels = desc.mipLevels,
            .arrayLayers = desc.arrayLayers,
            .samples = desc.samples,
            .tiling = desc.tiling,
            .usage = desc.usageFlags,
            .sharingMode = desc.sharingMode,
            .queueFamilyIndexCount = static_cast<uint32_t>(desc.queueFamilyIndices.size()),
            .pQueueFamilyIndices = desc.queueFamilyIndices.data(),
            .initialLayout = desc.imageLayout
        };

        vma::AllocationCreateInfo allocationCI{
            .flags = allocDesc.flags,
            .usage = allocDesc.usage,
            .priority = allocDesc.priority
        };

        auto result = vmaAllocator.createImage(&imageCI, &allocationCI, &image, &vmaAllocation, &vmaAllocationInfo);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create Vulkan image: " + vk::to_string(result));
        }

        vk::ImageViewCreateInfo imageViewCI{
            .flags = {},
            .image = image,
            .viewType = desc.viewType,
            .format = desc.format,
            .subresourceRange = {
                .aspectMask = desc.aspectMask,
                .baseMipLevel = 0,
                .levelCount = desc.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
            }
        };

        imageView = device.createImageView(imageViewCI);
    }

    VulkanTexture::~VulkanTexture()
    {
        device.destroyImageView(imageView);
        vmaAllocator.destroyImage(image, vmaAllocation);
    }

    void VulkanTexture::TransitionLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout layout)
    {
        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = {},
            .oldLayout = desc.imageLayout,
            .newLayout = layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = desc.aspectMask,
                .baseMipLevel = 0,
                .levelCount = desc.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
            }
        };

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, 0, nullptr, 0, nullptr, 1, &barrier);
        desc.imageLayout = layout;
    }
}
