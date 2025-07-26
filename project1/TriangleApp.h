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
        virtual bool Initialize() override;
        virtual void Render(vk::CommandBuffer commandBuffer) override;

    private:
        vk::Pipeline pipeline{};
    };
}