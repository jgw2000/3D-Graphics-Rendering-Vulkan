#pragma once

#include "Window.h"
#include "VulkanImgui.h"
#include "VulkanContext.h"

namespace jgw
{
    class BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(BaseApp)

        BaseApp(const WindowConfig& config = {});

        void Start();

    protected:
        virtual bool Initialize();
        virtual void Update();
        virtual void Render(vk::CommandBuffer commandBuffer) {}
        virtual void Cleanup();
        virtual void OnKey(int key, int scancode, int action, int mods) {}
        virtual void OnResize(int width, int height);
        virtual void CreateUI() {}

        virtual std::vector<const char*> GetInstanceLayers() const;
        virtual std::vector<const char*> GetInstanceExtensions() const;
        virtual std::vector<const char*> GetDeviceExtensions() const;

        vk::Device GetDevice() { return contextPtr->GetDevice(); }

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
        void SetCallback(GLFWwindow* handle);

        int iconified = 0;
    };
}
