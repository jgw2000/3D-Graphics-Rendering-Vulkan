#pragma once

#include "Common.h"

namespace jgw
{
    class VulkanBuffer final
    {
        friend class VulkanContext;

    public:
        CLASS_COPY_MOVE_DELETE(VulkanBuffer)

        explicit VulkanBuffer(
            vk::DeviceSize size,
            vk::BufferUsageFlags bufferUsage,
            vma::Allocator vmaAllocator,
            vma::AllocationCreateFlags flags = {},
            vma::MemoryUsage memoryUsage = vma::MemoryUsage::eAuto
        );

        ~VulkanBuffer();

        void Map();
        void CopyFromHost(void* data, vk::DeviceSize size);

        vk::Buffer Handle() { return buffer; }
        vk::DeviceSize TotalSize() { return size; }

    private:
        vk::DeviceSize size;
        vk::Buffer buffer;
        vk::BufferUsageFlags bufferUsage;

        vma::Allocator vmaAllocator;
        vma::Allocation vmaAllocation;
        vma::AllocationInfo vmaAllocationInfo;
        vma::MemoryUsage memoryUsage;

        mutable void* mappedMemory{ nullptr };
    };
}
