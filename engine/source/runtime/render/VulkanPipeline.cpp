#include "VulkanPipeline.h"

namespace jgw
{
    VulkanPipeline::VulkanPipeline(vk::Device device, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout) :
        device(device),
        pipeline(pipeline),
        pipelineLayout(pipelineLayout)
    {

    }

    VulkanPipeline::~VulkanPipeline()
    {
        if (device)
        {
            device.destroyPipelineLayout(pipelineLayout);
            device.destroyPipeline(pipeline);
        }
    }
}
