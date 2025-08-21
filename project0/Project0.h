#pragma once

#include "BaseApp.h"

namespace jgw
{
    class Project0 : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(Project0)

        Project0(const WindowConfig& config = {});

    protected:
        virtual bool OnInit() override;
        virtual void OnRender(vk::CommandBuffer commandBuffer) override;
        virtual void OnCleanup() override;

    private:
        std::unique_ptr<VulkanPipeline> pipeline;
    };
}