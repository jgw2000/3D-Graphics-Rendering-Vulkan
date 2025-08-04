#pragma once

#include "Common.h"

namespace jgw
{
    class VulkanPipeline final
    {
    public:
        CLASS_COPY_MOVE_DELETE(VulkanPipeline)

        explicit VulkanPipeline(vk::Device device, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout);
        ~VulkanPipeline();

        vk::Pipeline Handle() const { return pipeline; }
        vk::PipelineLayout Layout() const { return pipelineLayout; }

    private:
        vk::Device device;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;
    };
}
