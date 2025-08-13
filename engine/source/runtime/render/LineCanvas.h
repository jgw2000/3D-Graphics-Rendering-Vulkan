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
    class LineCanvas3D
    {
    public:
        CLASS_COPY_MOVE_DELETE(LineCanvas3D)

        LineCanvas3D() = default;

        bool Initialize(VulkanContext& context);
        void Render(VulkanContext& context);

        void Clear();
        void Line(const glm::vec3 p1, const glm::vec3& p2, const glm::vec4& c);

    private:
        struct LineData
        {
            glm::vec4 pos;
            glm::vec4 color;
        };

        struct PushConstantData
        {
            glm::mat4 mvp;
            vk::DeviceAddress addr;
        } pushConstantData;

        std::vector<LineData> lines;
        std::unique_ptr<VulkanBuffer> lineBuffer;
        std::unique_ptr<VulkanPipeline> linePipeline;
    };

    class LinePipelineBuilder final : public PipelineBuilder
    {
    public:
        CLASS_COPY_MOVE_DELETE(LinePipelineBuilder)

        LinePipelineBuilder() = default;

        virtual vk::PipelineInputAssemblyStateCreateInfo BuildInputAssemblyState() override;
        virtual vk::PipelineRasterizationStateCreateInfo BuildRasterizationState() override;
    };
}
