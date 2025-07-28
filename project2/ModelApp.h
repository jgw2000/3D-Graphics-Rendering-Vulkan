#pragma once

#include "BaseApp.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

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

    private:
        bool LoadModel();
        bool CreatePipeline();
    };
}
