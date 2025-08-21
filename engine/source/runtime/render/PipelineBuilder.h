#pragma once

#include "Common.h"

namespace jgw
{
    class PipelineBuilder
    {
    public:
        CLASS_COPY_MOVE_DELETE(PipelineBuilder)

        PipelineBuilder() = default;

        std::vector<vk::PipelineShaderStageCreateInfo> BuildShaderStages(const vk::Device& device);

        void AddShader(vk::ShaderStageFlagBits flag, std::string filename)
        {
            shaderFiles.emplace(flag, filename);
        }

        void SetVertexBindingDescriptions(std::vector<vk::VertexInputBindingDescription>& descriptions);
        void SetVertexAttributeDescriptions(std::vector<vk::VertexInputAttributeDescription>& descriptions);
        void SetColorBlendAttachments(std::vector<vk::PipelineColorBlendAttachmentState>& states);
        void SetDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts);
        void SetPushConstantRanges(std::vector<vk::PushConstantRange>& ranges);

        vk::PipelineVertexInputStateCreateInfo& VertexInputStateCI() { return vertexInputStateCI; }
        vk::PipelineInputAssemblyStateCreateInfo& InputAssemblyCI() { return inputAssemblyStateCI; }
        vk::PipelineTessellationStateCreateInfo& TessellationStateCI() { return tessellationStateCI; }
        vk::PipelineViewportStateCreateInfo& ViewportStateCI() { return viewportStateCI; }
        vk::PipelineRasterizationStateCreateInfo& RasterizationStateCI() { return rasterizationStateCI; }
        vk::PipelineMultisampleStateCreateInfo& MultisampleStateCI() { return multisampleStateCI; }
        vk::PipelineDepthStencilStateCreateInfo& DepthStencilStateCI() { return depthStencilStateCI; }
        vk::PipelineColorBlendStateCreateInfo& ColorBlendStateCI() { return colorBlendStateCI; }
        vk::PipelineDynamicStateCreateInfo& DynamicStateCI() { return dynamicStateCI; }
        vk::PipelineLayoutCreateInfo& PipelineLayoutCI() { return layoutCI; }

    private:
        std::unordered_map<vk::ShaderStageFlagBits, std::string> shaderFiles;
        vk::ShaderModule LoadShader(const char* filename, const vk::Device& device);

        vk::PipelineVertexInputStateCreateInfo vertexInputStateCI{
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
        };

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{ 
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = vk::False
        };

        vk::PipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateCI{
            .domainOrigin = vk::TessellationDomainOrigin::eUpperLeft
        };

        vk::PipelineTessellationStateCreateInfo tessellationStateCI{
            .pNext = &tessellationDomainOriginStateCI
        };

        vk::PipelineViewportStateCreateInfo viewportStateCI{
            .viewportCount = 1,
            .pViewports = nullptr, // Viewports will be set dynamically
            .scissorCount = 1,
            .pScissors = nullptr   // Scissors will be set dynamically
        };

        vk::PipelineRasterizationStateCreateInfo rasterizationStateCI{
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        vk::PipelineMultisampleStateCreateInfo multisampleStateCI{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = vk::False,
            .alphaToOneEnable = vk::False
        };

        vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI{
            .depthTestEnable = vk::True,
            .depthWriteEnable = vk::True,
            .depthCompareOp = vk::CompareOp::eLess,
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
            .blendEnable = vk::True,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlendStateCI{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eClear,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState
        };

        std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicStateCI{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        vk::PipelineLayoutCreateInfo layoutCI{
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
        };
    };
}
