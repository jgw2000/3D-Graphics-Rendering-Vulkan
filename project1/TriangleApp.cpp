#include "TriangleApp.h"

namespace jgw
{
    TriangleApp::TriangleApp(const WindowConfig& config) : BaseApp(config)
    {

    }

    bool TriangleApp::OnInit()
    {
        PipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "shaders/triangle.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "shaders/triangle.frag.spv");
        pipeline = contextPtr->CreateGraphicsPipeline(pd);

        if (pipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void TriangleApp::OnRender(vk::CommandBuffer commandBuffer)
    {
        vk::ClearValue clear_value{
            .color = std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})
        };

        vk::RenderingAttachmentInfo colorAttachment{
            .imageView = contextPtr->GetSwapchain()->GetImageView(),
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clear_value
        };

        vk::RenderingInfo renderInfo{
            .renderArea = {.offset = {0, 0}, .extent = contextPtr->GetSwapchain()->GetExtent() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        };

        commandBuffer.beginRendering(renderInfo);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Handle());

        auto extent = contextPtr->GetSwapchain()->GetExtent();
        vk::Viewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        commandBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissor{
            .offset = {0, 0},
            .extent = extent
        };
        commandBuffer.setScissor(0, 1, &scissor);

        commandBuffer.draw(3, 1, 0, 0); // Draw a triangle

        imguiPtr->Render(commandBuffer);

        commandBuffer.endRendering();
    }

    void TriangleApp::OnCleanup()
    {
        pipeline.reset();
    }
}