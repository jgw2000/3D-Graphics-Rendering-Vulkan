#pragma once

#include "BaseApp.h"

namespace jgw
{
    class TriangleApp : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(TriangleApp)

        TriangleApp(const WindowConfig& config = {});

    protected:
        virtual bool OnInit() override;
        virtual void OnRender(vk::CommandBuffer commandBuffer) override;
        virtual void OnCleanup() override;

    private:
        std::unique_ptr<VulkanPipeline> pipeline;
    };
}