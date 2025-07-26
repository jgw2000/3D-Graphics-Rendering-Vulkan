#pragma once

#include "Macro.h"

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include <string>
#include <vector>

namespace jgw
{
    class IPipelineBuilder
    {
    public:
        CLASS_COPY_MOVE_DELETE(IPipelineBuilder)

        IPipelineBuilder() = default;

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

        void SetVertexShaderFile(std::string filename);
        void SetFragmentShaderFile(std::string filename);

    protected:
        bool hasVertexShader = true;
        bool hasFragmentShader = true;

        std::string vertexShaderFile;
        std::string fragmentShaderFile;

    private:
        vk::ShaderModule LoadShader(const char* filename, const vk::Device& device);

        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        std::vector<vk::DynamicState> dynamicStates;
    };
}
