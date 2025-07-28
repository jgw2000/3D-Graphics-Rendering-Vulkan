#pragma once

#include "BaseApp.h"

namespace jgw
{
    class ModelApp : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(ModelApp)

        ModelApp(const WindowConfig& config = {});

    protected:
        virtual bool Initialize() override;
        virtual void Render(vk::CommandBuffer commandBuffer) override;
    };
}