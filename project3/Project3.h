#pragma once

#include "BaseApp.h"
#include "scene/VulkanMesh.h"

namespace jgw
{
    class Project3 : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(Project3)

        Project3(const WindowConfig& config = {});

    protected:
        virtual bool OnInit() override;
        virtual void OnUpdate(double delta) override;
        virtual void OnRender(vk::CommandBuffer commandBuffer) override;
        virtual void OnCleanup() override;
        virtual void OnResize(int width, int height) override;

    private:
        bool LoadScene();
        bool CreatePipeline();
        void SetupCamera();

        std::unique_ptr<VulkanMesh> scene;
    };
}