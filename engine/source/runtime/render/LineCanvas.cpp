#include "LineCanvas.h"

namespace jgw
{
    bool LineCanvas3D::Initialize(VulkanContext& context)
    {
        LinePipelineBuilder pd;

        //linePipeline = context.CreateGraphicsPipeline(pd);
        //if (linePipeline == nullptr)
        //{
        //    return false;
        //}

        return true;
    }

    void LineCanvas3D::Render(VulkanContext& context)
    {
        if (lines.empty())
            return;

        vk::DeviceSize requiredSize = lines.size() * sizeof(LineData);
        if (!lineBuffer || lineBuffer->TotalSize() < requiredSize)
        {
            lineBuffer = context.CreateBuffer(
                requiredSize,
                vk::BufferUsageFlagBits::eStorageBuffer,
                vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
            );
            lineBuffer->Map();
        }

        lineBuffer->CopyFromHost(lines.data(), requiredSize);

        auto commandBuffer = context.GetCommandBuffer();
        //commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, linePipeline->Handle());
        //commandBuffer.draw(lines.size(), 1, 0, 0);
    }

    void LineCanvas3D::Clear()
    {
        lines.clear();
    }

    void LineCanvas3D::Line(const glm::vec3 p1, const glm::vec3& p2, const glm::vec4& c)
    {
        lines.push_back({ .pos = glm::vec4(p1, 1.0f), .color = c });
        lines.push_back({ .pos = glm::vec4(p2, 1.0f), .color = c });
    }

    vk::PipelineInputAssemblyStateCreateInfo LinePipelineBuilder::BuildInputAssemblyState()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{
            .topology = vk::PrimitiveTopology::eLineList,
            .primitiveRestartEnable = vk::False
        };

        return inputAssemblyStateCI;
    }

    vk::PipelineRasterizationStateCreateInfo LinePipelineBuilder::BuildRasterizationState()
    {
        vk::PipelineRasterizationStateCreateInfo rasterizationStateCI{
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eLine,
            .cullMode = vk::CullModeFlagBits::eNone,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        return rasterizationStateCI;
    }
}
