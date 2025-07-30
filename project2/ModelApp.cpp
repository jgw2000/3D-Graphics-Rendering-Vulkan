#include "ModelApp.h"

#include <spdlog/spdlog.h>
#include <vector>

namespace jgw
{
    ModelApp::ModelApp(const WindowConfig& config) : BaseApp(config)
    {

    }

    bool ModelApp::Initialize()
    {
        if (!BaseApp::Initialize())
            return false;

        if (!LoadModel())
        {
            spdlog::error("Failed to load model");
            return false;
        }

        if (!CreatePipeline())
        {
            spdlog::error("Failed to create pipeline");
            return false;
        }

        return true;
    }

    void ModelApp::Render(vk::CommandBuffer commandBuffer)
    {
        TransitionImageLayout(
            commandBuffer,
            contextPtr->GetSwapchain()->GetImage(),
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        );

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

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        vk::Buffer vertexBuffers[] = { vertexBuffer->Handle() };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(indexBuffer->Handle(), 0, vk::IndexType::eUint32);

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

        commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

        commandBuffer.endRendering();

        TransitionImageLayout(
            commandBuffer,
            contextPtr->GetSwapchain()->GetImage(),
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe
        );
    }

    void ModelApp::Cleanup()
    {
        vertexBuffer.reset();
        indexBuffer.reset();

        BaseApp::Cleanup();
    }

    bool ModelApp::LoadModel()
    {
        const aiScene* scene = aiImportFile("rubber_duck/scene.gltf", aiProcess_Triangulate);
        if (!scene || !scene->HasMeshes())
        {
            spdlog::error(aiGetErrorString());
            return false;
        }

        const aiMesh* mesh = scene->mMeshes[0];
        
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            const aiVector3D& vertex = mesh->mVertices[i];
            positions.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
                indices.push_back(mesh->mFaces[i].mIndices[j]);
        }

        aiReleaseImport(scene);

        // Vertex Buffer
        std::unique_ptr<VulkanBuffer> stagingVertexBuffer = contextPtr->CreateBuffer(
            sizeof(glm::vec3) * positions.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );
        
        vertexBuffer = contextPtr->CreateBuffer(
            sizeof(glm::vec3) * positions.size(),
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

        contextPtr->UploadBuffer(positions.data(), stagingVertexBuffer.get(), vertexBuffer.get());
        contextPtr->WaitQueueIdle();

        // Index Buffer
        std::unique_ptr<VulkanBuffer> stagingIndexBuffer = contextPtr->CreateBuffer(
            sizeof(uint32_t) * indices.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        indexBuffer = contextPtr->CreateBuffer(
            sizeof(uint32_t) * indices.size(),
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

        contextPtr->UploadBuffer(indices.data(), stagingIndexBuffer.get(), indexBuffer.get());
        contextPtr->WaitQueueIdle();

        return true;
    }

    bool ModelApp::CreatePipeline()
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {
            { .binding = 0, .stride = sizeof(glm::vec3), .inputRate = vk::VertexInputRate::eVertex}
        };

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
            { .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = 0 }
        };

        IPipelineBuilder pd;
        pd.SetVertexBindingDescriptions(bindingDescriptions);
        pd.SetVertexAttributeDescriptions(attributeDescriptions);
        pd.SetVertexShaderFile("shaders/model.vert.spv");
        pd.SetFragmentShaderFile("shaders/model.frag.spv");

        pipeline = contextPtr->CreateGraphicsPipeline(pd);
        if (pipeline == nullptr)
        {
            return false;
        }

        return true;
    }
}
