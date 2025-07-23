#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "VulkanSwapchain.h"

namespace jgw
{
    VulkanSwapchain::VulkanSwapchain(vk::PhysicalDevice& physicalDevice, vk::Device& device, vk::SurfaceKHR& surface)
        : physicalDevice{ physicalDevice }
        , device{ device }
        , surface{ surface }
    {
        surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
        surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);
        surfaceFormat = ChooseSurfaceFormat();
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        Destroy();
    }

    bool VulkanSwapchain::Create(GLFWwindow* handle)
    {
        vk::SwapchainCreateInfoKHR swapchainCI{
            .surface = surface,
            .minImageCount = ChooseMinImageCount(),
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = GetSurfaceExtent(handle),
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = surfaceCaps.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = ChoosePresentMode(),
            .clipped = vk::True,
            .oldSwapchain = swapchain
        };

        swapchain = device.createSwapchainKHR(swapchainCI);
        swapchainImages = device.getSwapchainImagesKHR(swapchain);
        return true;
    }

    void VulkanSwapchain::Destroy()
    {
        if (device && swapchain)
        {
            device.destroySwapchainKHR(swapchain);
            swapchain = nullptr;
            swapchainImages.clear();
        }
    }

    uint32_t VulkanSwapchain::ChooseMinImageCount() const
    {
        uint32_t imageCount = surfaceCaps.minImageCount + 1;
        if (imageCount > surfaceCaps.maxImageCount)
            imageCount = surfaceCaps.maxImageCount;
        return imageCount;
    }

    vk::SurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat() const
    {
        std::vector<vk::Format> preferredImageFormats = {
            vk::Format::eB8G8R8A8Srgb,
            vk::Format::eR8G8B8A8Srgb,
        };

        for (auto& availableFormat : surfaceFormats)
        {
            if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end())
            {
                return availableFormat;
            }
        }

        return surfaceFormats[0];
    }

    vk::Extent2D VulkanSwapchain::GetSurfaceExtent(GLFWwindow* handle) const
    {
        if (surfaceCaps.currentExtent.width == (uint32_t) - 1 || surfaceCaps.currentExtent.height == (uint32_t) - 1)
        {
            int width, height;
            glfwGetFramebufferSize(handle, &width, &height);
            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }
        return surfaceCaps.currentExtent;
    }

    vk::PresentModeKHR VulkanSwapchain::ChoosePresentMode() const
    {
        // Prefer mailbox mode for lower latency, fall back to FIFO if not available
        for (const auto& mode : surfacePresentModes)
        {
            if (mode == vk::PresentModeKHR::eMailbox)
                return mode;
        }
        return vk::PresentModeKHR::eFifo;
    }
}
