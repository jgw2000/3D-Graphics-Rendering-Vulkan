#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "IPipelineBuilder.h"

#define FMT_UNICODE 0
#include <spdlog/spdlog.h>
#include <fstream>

namespace jgw
{
    std::vector<vk::PipelineShaderStageCreateInfo> IPipelineBuilder::BuildShaderStages(const vk::Device& device)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        if (hasVertexShader) shaderStages.push_back(BuildVertexShaderStage(device));
        if (hasFragmentShader) shaderStages.push_back(BuildFragmentShaderStage(device));
        return shaderStages;
    }

    vk::PipelineShaderStageCreateInfo IPipelineBuilder::BuildVertexShaderStage(const vk::Device& device)
    {
        vk::PipelineShaderStageCreateInfo shaderStageCI{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = LoadShader(vertexShaderFile.c_str(), device),
            .pName = "main"
        };

        return shaderStageCI;
    }

    vk::PipelineShaderStageCreateInfo IPipelineBuilder::BuildFragmentShaderStage(const vk::Device& device)
    {
        vk::PipelineShaderStageCreateInfo shaderStageCI{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = LoadShader(fragmentShaderFile.c_str(), device),
            .pName = "main"
        };

        return shaderStageCI;
    }

    vk::PipelineVertexInputStateCreateInfo IPipelineBuilder::BuildVertexInputState()
    {
        vk::PipelineVertexInputStateCreateInfo vertexInputStateCI{
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
        };

        return vertexInputStateCI;
    }

    vk::PipelineInputAssemblyStateCreateInfo IPipelineBuilder::BuildInputAssemblyState()
    {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = vk::False
        };
        
        return inputAssemblyStateCI;
    }

    vk::PipelineViewportStateCreateInfo IPipelineBuilder::BuildViewportState()
    {
        vk::PipelineViewportStateCreateInfo viewportStateCI{
            .viewportCount = 1,
            .pViewports = nullptr, // Viewports will be set dynamically
            .scissorCount = 1,
            .pScissors = nullptr   // Scissors will be set dynamically
        };
        return viewportStateCI;
    }

    vk::PipelineRasterizationStateCreateInfo IPipelineBuilder::BuildRasterizationState()
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

    vk::PipelineMultisampleStateCreateInfo IPipelineBuilder::BuildMultisampleState()
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

    vk::PipelineDepthStencilStateCreateInfo IPipelineBuilder::BuildDepthStencilState()
    {
        vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI{
            .depthTestEnable = vk::True,
            .depthWriteEnable = vk::True,
            .depthCompareOp = vk::CompareOp::eLess,
        };

        return depthStencilStateCI;
    }

    vk::PipelineColorBlendStateCreateInfo IPipelineBuilder::BuildColorBlendState()
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

    vk::PipelineDynamicStateCreateInfo IPipelineBuilder::BuildDynamicState()
    {
        dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        vk::PipelineDynamicStateCreateInfo dynamicStateCI{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        return dynamicStateCI;
    }

    vk::PipelineLayout IPipelineBuilder::BuildLayout(const vk::Device& device)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCI{
            .setLayoutCount = 0, // No descriptor sets
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0, // No push constants
            .pPushConstantRanges = nullptr
        };
        return device.createPipelineLayout(pipelineLayoutCI);
    }

    void IPipelineBuilder::SetVertexShaderFile(std::string filename)
    {
        vertexShaderFile = filename;
        hasVertexShader = true;
    }

    void IPipelineBuilder::SetFragmentShaderFile(std::string filename)
    {
        fragmentShaderFile = filename;
        hasFragmentShader = true;
    }

    vk::ShaderModule IPipelineBuilder::LoadShader(const char* filename, const vk::Device& device)
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
