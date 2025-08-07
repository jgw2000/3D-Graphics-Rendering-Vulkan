#pragma once

#include "Window.h"
#include "VulkanContext.h"
#include "VulkanImgui.h"
#include "FpsCounter.h"

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_vulkan.h"

namespace jgw
{
    class BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(BaseApp)

        BaseApp(const WindowConfig& config = {});

        void Start();

    protected:
        virtual void Cleanup();
        virtual bool OnInit() { return true; }
        virtual void OnRender(vk::CommandBuffer commandBuffer) {}
        virtual void OnGUI() {}
        virtual void OnUpdate(double delta) {}
        virtual void OnKey(int key, int scancode, int action, int mods) {}
        virtual void OnResize(int width, int height);

        virtual std::vector<const char*> GetInstanceLayers() const;
        virtual std::vector<const char*> GetInstanceExtensions() const;
        virtual std::vector<const char*> GetDeviceExtensions() const;

        vk::Device GetDevice() { return contextPtr->GetDevice(); }

        bool InitImgui(vk::Format depthFormat);

        void TransitionImageLayout(
            vk::CommandBuffer       commandBuffer,
            vk::Image               image,
            vk::ImageLayout         oldLayout,
            vk::ImageLayout         newLayout,
            vk::AccessFlags         srcAccessMask,
            vk::AccessFlags         dstAccessMask,
            vk::PipelineStageFlags  srcStage,
            vk::PipelineStageFlags  dstStage
        );

        std::unique_ptr<VulkanTexture> LoadTexture(const char* filename, bool mipmapped = false);

        std::unique_ptr<Window> windowPtr;
        std::unique_ptr<VulkanContext> contextPtr;
        std::unique_ptr<VulkanImgui> imguiPtr;

    private:
        bool Initialize();
        void Render();
        void Update(double delta);
        void ShowFPS();
        void SetCallback(GLFWwindow* handle);

        int iconified = 0;

        FpsCounter fpsCounter{};
    };
}
