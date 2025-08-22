#pragma once

#include "Common.h"
#include "VulkanContext.h"

namespace jgw
{
    class GridCanvas
    {
    public:
        CLASS_COPY_MOVE_DELETE(GridCanvas)

        GridCanvas() = default;

        bool Initialize(VulkanContext& context);
        void Render(VulkanContext& context);

        void SetMatrix(glm::mat4 m);
        void SetCameraPos(glm::vec4 pos);

    private:
        struct PushConstantData
        {
            glm::mat4 mvp;
            glm::vec4 cameraPos;
        } pushConstantData;

        std::unique_ptr<VulkanPipeline> gridPipeline;
    };
}
