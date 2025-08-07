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
        imguiPtr = std::make_unique<VulkanImgui>();
    }

    void BaseApp::Start()
    {
        if (!Initialize())
            return;

        double lastTime = glfwGetTime();
        double curTime = 0.0;

        GLFWwindow* handle = windowPtr->GetHandle();
        while (!glfwWindowShouldClose(handle))
        {
            glfwPollEvents();

            curTime = glfwGetTime();
            Update(curTime - lastTime);
            lastTime = curTime;
            
            if (!iconified && contextPtr->BeginRender())
            {
                Render();
                contextPtr->EndRender();
            }
        }

        contextPtr->WaitDeviceIdle();
        Cleanup();
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

    bool BaseApp::InitImgui(vk::Format depthFormat)
    {
        return imguiPtr->Initialize(
            windowPtr->GetHandle(),
            contextPtr->GetInstance(),
            contextPtr->GetPhysicalDevice(),
            contextPtr->GetDevice(),
            contextPtr->GetQueue(),
            contextPtr->GetQueuFamily(),
            contextPtr->GetSwapchain()->GetImageCount(),
            static_cast<VkFormat>(contextPtr->GetSwapchain()->GetFormat()),
            static_cast<VkFormat>(depthFormat),
            contextPtr->GetApiVersion()
        );
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

    std::unique_ptr<VulkanTexture> BaseApp::CreateDepthTexture(vk::Format depthFormat)
    {
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const TextureDesc desc{
            .usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
            .format = depthFormat,
            .extent = {
                .width = extent.width, .height = extent.height, .depth = 1
            },
            .aspectMask = vk::ImageAspectFlagBits::eDepth
        };

        const VmaAllocationDesc allocDesc{
            .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
            .usage = vma::MemoryUsage::eAutoPreferDevice
        };

        return contextPtr->CreateTexture(desc, allocDesc);
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

        return OnInit();
    }

    void BaseApp::Render()
    {
        auto commandBuffer = contextPtr->GetCommandBuffer();

        TransitionImageLayout(
            commandBuffer,
            contextPtr->GetSwapchain()->GetImage(),
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        );

        OnRender(commandBuffer);

        TransitionImageLayout(
            commandBuffer,
            contextPtr->GetSwapchain()->GetImage(),
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe
        );
    }

    void BaseApp::Update(double delta)
    {
        fpsCounter.Tick(delta);

        OnUpdate(delta);

        imguiPtr->BeginFrame();
        OnGUI();
        ShowFPS();
        imguiPtr->EndFrame();
    }

    void BaseApp::ShowFPS()
    {
        int curFPS = (int)fpsCounter.GetFPS();
        if (curFPS <= 0)
            return;

        if (const ImGuiViewport* v = ImGui::GetMainViewport())
        {
            ImGui::SetNextWindowPos(
                { v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 15.0f },
                ImGuiCond_Always,
                { 1.0f, 0.0f }
            );
        }

        ImGui::SetNextWindowBgAlpha(0.3f);
        ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize("FPS : _______").x, 0));

        if (ImGui::Begin("##FPS", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove))
        {
            ImGui::Text("FPS : %i", curFPS);
            ImGui::Text("ms  : %.1f", 1000.0 / curFPS);
        }
        ImGui::End();
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
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));

            if (width <= 0 || height <= 0)
            {
                if (app) app->iconified = true;
                return;
            }

            if (app)
            {
                app->OnResize(width, height);
                app->iconified = false;
            }
        });

        glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
            BaseApp* app = static_cast<BaseApp*>(glfwGetWindowUserPointer(window));
            app->iconified = iconified;
        });
    }
}
