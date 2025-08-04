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
            .initialLayout = desc.initialLayout
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

    void VulkanTexture::TransitionLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = {},
            .oldLayout = oldLayout,
            .newLayout = newLayout,
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
    }

    void VulkanTexture::TransitionMipLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevel)
    {
        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = {},
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = desc.aspectMask,
                .baseMipLevel = mipLevel,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
            }
        };

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanTexture::GenerateMipmap(vk::CommandBuffer commandBuffer)
    {
        int32_t mipWidth = desc.extent.width;
        int32_t mipHeight = desc.extent.height;

        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask = vk::AccessFlagBits::eTransferRead,
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = vk::ImageLayout::eTransferSrcOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = desc.aspectMask,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
}
        };

        vk::ImageBlit blit{
            .srcSubresource = {
                .aspectMask = desc.aspectMask,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
            },
            .dstSubresource = {
                .aspectMask = desc.aspectMask,
                .baseArrayLayer = 0,
                .layerCount = desc.arrayLayers
            }
        };

        for (uint32_t i = 1; i < desc.mipLevels; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &barrier);
        
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            
            mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
            mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

            blit.dstSubresource.mipLevel = i;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
            commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);
        
            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &barrier);
        }
    }
}
