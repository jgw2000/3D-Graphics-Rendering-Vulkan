#pragma once

#include "Macro.h"
#include "Window.h"

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vector>

namespace jgw
{
    class VulkanSwapchain final
    {
    public:
        CLASS_COPY_MOVE_DELETE(VulkanSwapchain)

        VulkanSwapchain(vk::PhysicalDevice& physicalDevice, vk::Device& device, vk::SurfaceKHR& surface);
        ~VulkanSwapchain();

        bool Create(GLFWwindow* handle);
        void Destroy();

    private:
        uint32_t ChooseMinImageCount() const;
        vk::SurfaceFormatKHR ChooseSurfaceFormat() const;
        vk::Extent2D GetSurfaceExtent(GLFWwindow* handle) const;
        vk::PresentModeKHR ChoosePresentMode() const;

    private:
        vk::PhysicalDevice& physicalDevice;
        vk::Device& device;
        vk::SurfaceKHR& surface;

        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> surfacePresentModes;
        std::vector<vk::Image> swapchainImages;

        vk::SurfaceFormatKHR surfaceFormat;
        vk::SurfaceCapabilitiesKHR surfaceCaps{};
        vk::SwapchainKHR swapchain{};
    };
}
