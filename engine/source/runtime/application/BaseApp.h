#pragma once

#include "Window.h"
#include "VulkanContext.h"
#include "VulkanImgui.h"
#include "Camera.h"
#include "FpsCounter.h"
#include "LineCanvas.h"
#include "GridCanvas.h"

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_vulkan.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace jgw
{
    class BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(BaseApp)

        BaseApp(const WindowConfig& config = {});

        void Start();
        void Resize(int width, int height);

    protected:
        virtual bool OnInit() { return true; }
        virtual void OnRender(vk::CommandBuffer commandBuffer) {}
        virtual void OnCleanup() {}
        virtual void OnGUI() {}
        virtual void OnGizmos() {};
        virtual void OnUpdate(double delta) {}
        virtual void OnKey(int key, int scancode, int action, int mods);
        virtual void OnMouse(int button, int action, int modes);
        virtual void OnMouseMove(float x, float y);
        virtual void OnMouseScroll(float x, float y);
        virtual void OnResize(int width, int height) {}

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
        std::unique_ptr<VulkanTexture> LoadCubeTexture(const char* filename, vk::Format format);

        std::unique_ptr<Window> windowPtr;
        std::unique_ptr<VulkanContext> contextPtr;
        std::unique_ptr<VulkanImgui> imguiPtr;
        std::unique_ptr<Camera> cameraPtr;
        std::unique_ptr<LineCanvas3D> canvas3D;
        std::unique_ptr<LineCanvas2D> canvas2D;
        std::unique_ptr<GridCanvas> canvasGrid;

        struct MouseState
        {
            glm::vec2 pos = glm::vec2(0.0f);
            bool pressedLeft = false;
            bool pressedRight = false;
        } mouseState;

        FpsCounter fpsCounter{};

    private:
        bool Initialize();
        void Render();
        void Update(double delta);
        void Cleanup();
        void ShowFPS();
        void SetCallback(GLFWwindow* handle);

        int iconified = 0;
        bool showUI = true;

        
    };
}
