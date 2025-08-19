#pragma once

#include "Common.h"
#include "VulkanContext.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
