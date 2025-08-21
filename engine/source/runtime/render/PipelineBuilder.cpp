#include "PipelineBuilder.h"

#include <fstream>

namespace jgw
{
    std::vector<vk::PipelineShaderStageCreateInfo> PipelineBuilder::BuildShaderStages(const vk::Device& device)
    {
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        for (const auto& it : shaderFiles)
        {
            vk::PipelineShaderStageCreateInfo shaderStageCI{
                .stage = it.first,
                .module = LoadShader(it.second.c_str(), device),
                .pName = "main"
            };
            shaderStages.emplace_back(shaderStageCI);
        }
        return shaderStages;
    }

    void PipelineBuilder::SetVertexBindingDescriptions(std::vector<vk::VertexInputBindingDescription>& descriptions)
    {
        vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(descriptions.size());
        vertexInputStateCI.pVertexBindingDescriptions = descriptions.data();
    }

    void PipelineBuilder::SetVertexAttributeDescriptions(std::vector<vk::VertexInputAttributeDescription>& descriptions)
    {
        vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(descriptions.size());
        vertexInputStateCI.pVertexAttributeDescriptions = descriptions.data();
    }

    void PipelineBuilder::SetColorBlendAttachments(std::vector<vk::PipelineColorBlendAttachmentState>& states)
    {
        colorBlendStateCI.attachmentCount = static_cast<uint32_t>(states.size());
        colorBlendStateCI.pAttachments = states.data();
    }

    void PipelineBuilder::SetDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts)
    {
        layoutCI.setLayoutCount = static_cast<uint32_t>(layouts.size());
        layoutCI.pSetLayouts = layouts.data();
    }

    void PipelineBuilder::SetPushConstantRanges(std::vector<vk::PushConstantRange>& ranges)
    {
        layoutCI.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        layoutCI.pPushConstantRanges = ranges.data();
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
