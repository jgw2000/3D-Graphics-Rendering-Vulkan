#include "BaseApp.h"
#include "PipelineBuilder.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb.h>
#include <stb_image.h>
#include <array>

namespace jgw
{
    BaseApp::BaseApp(const WindowConfig& config)
    {
        windowPtr = std::make_unique<Window>(config);
        contextPtr = std::make_unique<VulkanContext>();
    }

    void BaseApp::Start()
    {
        if (!Initialize())
            return;

        GLFWwindow* handle = windowPtr->GetHandle();
        while (!glfwWindowShouldClose(handle))
        {
            glfwPollEvents();

            Update();
            
            if (!iconified && contextPtr->BeginRender())
            {
                Render(contextPtr->GetCommandBuffer());
                contextPtr->EndRender();
            }
        }

        contextPtr->WaitDeviceIdle();
        Cleanup();
    }

    bool BaseApp::Initialize()
    {
        if (!windowPtr->Initialize())
            return false;

        GLFWwindow* handle = windowPtr->GetHandle();
        SetCallback(handle);

        auto instanceLayers = GetInstanceLayers();
        auto instanceExtensions = GetInstanceExtensions();
        auto deviceExtensions = GetDeviceExtensions();

        if (!contextPtr->Initialize(handle, instanceLayers, instanceExtensions, deviceExtensions))
            return false;

        imguiPtr = std::make_unique<VulkanImgui>();

        return true;
    }

    void BaseApp::Update()
    {
        imguiPtr->Update();
    }

    void BaseApp::Cleanup()
    {
        imguiPtr.reset();
        contextPtr.reset();
        windowPtr.reset();
    }

    void BaseApp::OnResize(int width, int height)
    {
        contextPtr->WindowResize();
    }

    std::vector<const char*> BaseApp::GetInstanceLayers() const
    {
#if defined(_DEBUG)
        return { "VK_LAYER_KHRONOS_validation" };
#else
        return { };
#endif
    }

    std::vector<const char*> BaseApp::GetInstanceExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if defined(_DEBUG)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return extensions;
    }

    std::vector<const char*> BaseApp::GetDeviceExtensions() const
    {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
    }

    void BaseApp::TransitionImageLayout(
        vk::CommandBuffer       commandBuffer,
        vk::Image               image,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags         srcAccessMask,
        vk::AccessFlags         dstAccessMask,
        vk::PipelineStageFlags  srcStage,
        vk::PipelineStageFlags  dstStage
    )
    {
        vk::ImageMemoryBarrier imageMemoryBarrier{
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .image = image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        commandBuffer.pipelineBarrier(srcStage, dstStage, {}, {}, {}, imageMemoryBarrier);
    }

    std::unique_ptr<VulkanTexture> BaseApp::LoadTexture(const char* filename, bool mipmapped)
    {
        int w, h, comp;
        stbi_set_flip_vertically_on_load(true);
        auto* data = stbi_load(filename, &w, &h, &comp, 4);

        const TextureDesc desc{
            .usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            .extent = {
                .width = static_cast<uint32_t>(w),
                .height = static_cast<uint32_t>(h),
                .depth = 1
            },
            .mipLevels = mipmapped ? static_cast<uint32_t>(std::floor(std::log2(std::max<int>(w, h)))) + 1 : 1,
            .mipmapped = mipmapped
        };

        const VmaAllocationDesc allocDesc{
            .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
            .usage = vma::MemoryUsage::eAutoPreferDevice
        };

        std::unique_ptr<VulkanTexture> texture = contextPtr->CreateTexture(desc, allocDesc);
        std::unique_ptr<VulkanBuffer> stagingBuffer = contextPtr->CreateBuffer(
            w * h * 4,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        // TODO
        contextPtr->BeginCommand();
        contextPtr->UploadTexture(data, stagingBuffer.get(), texture.get());
        contextPtr->EndCommand();

        stbi_image_free(data);

        return texture;
    }

    void BaseApp::SetCallback(GLFWwindow* handle)
    {
        glfwSetWindowUserPointer(handle, this);

        glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnKey(key, scancode, action, mods);

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        });

        glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
            if (width <= 0 || height <= 0)
                return;

            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            if (app) app->OnResize(width, height);
        });

        glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            app->iconified = iconified;
        });
    }
}
