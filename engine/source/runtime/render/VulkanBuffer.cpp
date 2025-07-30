#include "VulkanBuffer.h"

#include <cassert>

namespace jgw
{
    VulkanBuffer::VulkanBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags bufferUsage,
        vma::Allocator vmaAllocator,
        vma::AllocationCreateFlags flags,
        vma::MemoryUsage memoryUsage
    ):
        vmaAllocator(vmaAllocator),
        size(size),
        bufferUsage(bufferUsage),
        memoryUsage(memoryUsage)
    {
        vk::BufferCreateInfo bufferCI{
            .size = size,
            .usage = bufferUsage
        };

        vma::AllocationCreateInfo allocationCI{
            .flags = flags,
            .usage = memoryUsage,
        };

        auto result = vmaAllocator.createBuffer(&bufferCI, &allocationCI, &buffer, &vmaAllocation, &vmaAllocationInfo);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create Vulkan buffer: " + vk::to_string(result));
        }

        if (bufferUsage & vk::BufferUsageFlagBits::eTransferSrc)
        {
            mappedMemory = vmaAllocator.mapMemory(vmaAllocation);
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (mappedMemory)
        {
            vmaAllocator.unmapMemory(vmaAllocation);
            mappedMemory = nullptr;
        }

        vmaAllocator.destroyBuffer(buffer, vmaAllocation);
    }
}
