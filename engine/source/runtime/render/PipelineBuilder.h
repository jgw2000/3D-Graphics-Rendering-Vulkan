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
        virtual vk::PipelineShaderStageCreateInfo BuildVertexShaderStage(const vk::Device& device);
        virtual vk::PipelineShaderStageCreateInfo BuildFragmentShaderStage(const vk::Device& device);
        virtual vk::PipelineVertexInputStateCreateInfo BuildVertexInputState();
        virtual vk::PipelineInputAssemblyStateCreateInfo BuildInputAssemblyState();
        virtual vk::PipelineViewportStateCreateInfo BuildViewportState();
        virtual vk::PipelineRasterizationStateCreateInfo BuildRasterizationState();
        virtual vk::PipelineMultisampleStateCreateInfo BuildMultisampleState();
        virtual vk::PipelineDepthStencilStateCreateInfo BuildDepthStencilState();
        virtual vk::PipelineColorBlendStateCreateInfo BuildColorBlendState();
        virtual vk::PipelineDynamicStateCreateInfo BuildDynamicState();
        virtual vk::PipelineLayout BuildLayout(const vk::Device& device);
        virtual vk::PipelineRenderingCreateInfo BuildRendering();

        void SetVertexShaderFile(std::string filename);
        void SetFragmentShaderFile(std::string filename);

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
        void SetColorFormats(std::vector<vk::Format>& formats)
        {
            colorFormats = formats;
        }
        void SetDepthFormat(vk::Format format)
        {
            depthFormat = format;
        }

    protected:
        bool hasVertexShader = true;
        bool hasFragmentShader = true;

        std::string vertexShaderFile;
        std::string fragmentShaderFile;

    private:
        vk::ShaderModule LoadShader(const char* filename, const vk::Device& device);

        std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        std::vector<vk::DynamicState> dynamicStates;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        std::vector<vk::PushConstantRange> pushConstantRanges;
        std::vector<vk::Format> colorFormats;
        vk::Format depthFormat = vk::Format::eUndefined;
    };
}
