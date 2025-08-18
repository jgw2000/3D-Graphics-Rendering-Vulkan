#pragma once

#include "Common.h"

namespace jgw
{
    class PipelineBuilder
    {
    public:
        CLASS_COPY_MOVE_DELETE(PipelineBuilder)

        PipelineBuilder() = default;

        virtual std::vector<vk::PipelineShaderStageCreateInfo> BuildShaderStages(const vk::Device& device);
        virtual vk::PipelineVertexInputStateCreateInfo BuildVertexInputState();
        virtual vk::PipelineInputAssemblyStateCreateInfo BuildInputAssemblyState();
        virtual vk::PipelineViewportStateCreateInfo BuildViewportState();
        virtual vk::PipelineRasterizationStateCreateInfo BuildRasterizationState();
        virtual vk::PipelineMultisampleStateCreateInfo BuildMultisampleState();
        virtual vk::PipelineDepthStencilStateCreateInfo BuildDepthStencilState();
        virtual vk::PipelineColorBlendStateCreateInfo BuildColorBlendState();
        virtual vk::PipelineDynamicStateCreateInfo BuildDynamicState();
        virtual vk::PipelineLayout BuildLayout(const vk::Device& device);

        void AddShader(vk::ShaderStageFlagBits flag, std::string filename)
        {
            shaderFiles.emplace(flag, filename);
        }
        void SetVertexBindingDescriptions(std::vector<vk::VertexInputBindingDescription>& descriptions)
        {
            vertexBindingDescriptions = descriptions;
        }
        void SetVertexAttributeDescriptions(std::vector<vk::VertexInputAttributeDescription>& descriptions)
        {
            vertexAttributeDescriptions = descriptions;
        }
        void SetDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts)
        {
            descriptorSetLayouts = layouts;
        }
        void SetPushConstantRanges(std::vector<vk::PushConstantRange>& ranges)
        {
            pushConstantRanges = ranges;
        }

    private:
        std::unordered_map<vk::ShaderStageFlagBits, std::string> shaderFiles;
        vk::ShaderModule LoadShader(const char* filename, const vk::Device& device);

        std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        std::vector<vk::DynamicState> dynamicStates;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        std::vector<vk::PushConstantRange> pushConstantRanges;
    };

    class WireframePipelineBuilder : public PipelineBuilder
    {
    public:
        CLASS_COPY_MOVE_DELETE(WireframePipelineBuilder)

        WireframePipelineBuilder() = default;

        virtual vk::PipelineRasterizationStateCreateInfo BuildRasterizationState() override;
    };
}
