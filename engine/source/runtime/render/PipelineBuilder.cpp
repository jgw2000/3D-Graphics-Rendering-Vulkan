#include "PipelineBuilder.h"

#include <fstream>

namespace jgw
{
    std::vector<vk::PipelineShaderStageCreateInfo> PipelineBuilder::BuildShaderStages(const vk::Device& device)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        if (hasVertexShader) shaderStages.push_back(BuildVertexShaderStage(device));
        if (hasFragmentShader) shaderStages.push_back(BuildFragmentShaderStage(device));
        return shaderStages;
    }

    vk::PipelineShaderStageCreateInfo PipelineBuilder::BuildVertexShaderStage(const vk::Device& device)
    {
        vk::PipelineShaderStageCreateInfo shaderStageCI{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = LoadShader(vertexShaderFile.c_str(), device),
            .pName = "main"
        };

        return shaderStageCI;
    }

    vk::PipelineShaderStageCreateInfo PipelineBuilder::BuildFragmentShaderStage(const vk::Device& device)
    {
        vk::PipelineShaderStageCreateInfo shaderStageCI{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = LoadShader(fragmentShaderFile.c_str(), device),
            .pName = "main"
        };

        return shaderStageCI;
    }

    vk::PipelineVertexInputStateCreateInfo PipelineBuilder::BuildVertexInputState()
    {
        vk::PipelineVertexInputStateCreateInfo vertexInputStateCI{
            .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size()),
            .pVertexBindingDescriptions = vertexBindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = vertexAttributeDescriptions.data()
        };

        return vertexInputStateCI;
    }

    vk::PipelineInputAssemblyStateCreateInfo PipelineBuilder::BuildInputAssemblyState()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = vk::False
        };
        
        return inputAssemblyStateCI;
    }

    vk::PipelineViewportStateCreateInfo PipelineBuilder::BuildViewportState()
    {
        vk::PipelineViewportStateCreateInfo viewportStateCI{
            .viewportCount = 1,
            .pViewports = nullptr, // Viewports will be set dynamically
            .scissorCount = 1,
            .pScissors = nullptr   // Scissors will be set dynamically
        };
        return viewportStateCI;
    }

    vk::PipelineRasterizationStateCreateInfo PipelineBuilder::BuildRasterizationState()
    {
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

        return rasterizationStateCI;
    }

    vk::PipelineMultisampleStateCreateInfo PipelineBuilder::BuildMultisampleState()
    {
        vk::PipelineMultisampleStateCreateInfo multisampleStateCI{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = vk::False,
            .alphaToOneEnable = vk::False
        };

        return multisampleStateCI;
    }

    vk::PipelineDepthStencilStateCreateInfo PipelineBuilder::BuildDepthStencilState()
    {
        vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI{
            .depthTestEnable = vk::True,
            .depthWriteEnable = vk::True,
            .depthCompareOp = vk::CompareOp::eLess,
        };

        return depthStencilStateCI;
    }

    vk::PipelineColorBlendStateCreateInfo PipelineBuilder::BuildColorBlendState()
    {
        colorBlendAttachmentStates.push_back(
            vk::PipelineColorBlendAttachmentState{
                .blendEnable = vk::True,
                .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                .alphaBlendOp = vk::BlendOp::eAdd,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            }
        );

        vk::PipelineColorBlendStateCreateInfo colorBlendStateCI{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eClear,
            .attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size()),
            .pAttachments = colorBlendAttachmentStates.data()
        };

        return colorBlendStateCI;
    }

    vk::PipelineDynamicStateCreateInfo PipelineBuilder::BuildDynamicState()
    {
        dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        vk::PipelineDynamicStateCreateInfo dynamicStateCI{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        return dynamicStateCI;
    }

    vk::PipelineLayout PipelineBuilder::BuildLayout(const vk::Device& device)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCI{
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
            .pPushConstantRanges = pushConstantRanges.data()
        };

        return device.createPipelineLayout(pipelineLayoutCI);
    }

    vk::PipelineRenderingCreateInfo PipelineBuilder::BuildRendering()
    {
        vk::PipelineRenderingCreateInfo renderingCI{
            .colorAttachmentCount = static_cast<uint32_t>(colorFormats.size()),
            .pColorAttachmentFormats = colorFormats.data(),
            .depthAttachmentFormat = depthFormat
        };

        return renderingCI;
    }

    void PipelineBuilder::SetVertexShaderFile(std::string filename)
    {
        vertexShaderFile = filename;
        hasVertexShader = true;
    }

    void PipelineBuilder::SetFragmentShaderFile(std::string filename)
    {
        fragmentShaderFile = filename;
        hasFragmentShader = true;
    }

    vk::ShaderModule PipelineBuilder::LoadShader(const char* filename, const vk::Device& device)
    {
        std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

        if (is.is_open())
        {
            size_t size = is.tellg();
            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size];
            is.read(shaderCode, size);
            is.close();

            vk::ShaderModuleCreateInfo shaderModuleCI{
                .codeSize = size,
                .pCode = (uint32_t*)shaderCode
            };

            vk::ShaderModule shaderModule = device.createShaderModule(shaderModuleCI);

            delete[] shaderCode;
            return shaderModule;
        }
        else
        {
            spdlog::error("Could not open shader file {}", filename);
            return nullptr;
        }
    }
}
