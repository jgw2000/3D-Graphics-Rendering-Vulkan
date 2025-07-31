#include "VulkanTexture.h"

namespace jgw
{
    VulkanTexture::VulkanTexture(vk::Device device, vma::Allocator allocator, const TextureDesc& desc, const VmaAllocationDesc& allocDesc)
        : device(device)
        , vmaAllocator(allocator)
        , imageType(desc.imageType)
        , format(desc.format)
        , extent(desc.extent)
    {
        vk::ImageCreateInfo imageCI{
            .flags = desc.flags,
            .imageType = imageType,
            .format = format,
            .extent = extent,
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
}
