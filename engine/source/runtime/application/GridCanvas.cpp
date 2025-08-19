#include "GridCanvas.h"

namespace jgw
{
    bool GridCanvas::Initialize(VulkanContext& context)
    {
        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex, .offset = 0, .size = sizeof(PushConstantData) }
        };

        PipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "../engine/shaders/grid.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "../engine/shaders/grid.frag.spv");
        pd.SetPushConstantRanges(pushConstantRanges);

        gridPipeline = context.CreateGraphicsPipeline(pd);
        if (gridPipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void GridCanvas::Render(VulkanContext& context)
    {
        auto commandBuffer = context.GetCommandBuffer();
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, gridPipeline->Handle());
        commandBuffer.pushConstants(gridPipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData), &pushConstantData);
        commandBuffer.draw(6, 1, 0, 0);
    }

    void GridCanvas::SetMatrix(glm::mat4 m)
    {
        pushConstantData.mvp = m;
    }

    void GridCanvas::SetCameraPos(glm::vec4 pos)
    {
        pushConstantData.cameraPos = pos;
    }
}
