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

        if (!CreateDepthBuffer())
        {
            spdlog::error("Failed to create depth buffer");
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

        vk::ClearValue colorCV{
            .color = std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}),
        };

        vk::RenderingAttachmentInfo colorAttachment{
            .imageView = contextPtr->GetSwapchain()->GetImageView(),
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = colorCV
        };

        vk::ClearValue depthCV{
            .depthStencil = { .depth = 1.0f }
        };

        vk::RenderingAttachmentInfo depthAttachment{
            .imageView = depthTexture->GetView(),
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = depthCV
        };

        vk::RenderingInfo renderInfo{
            .renderArea = {.offset = {0, 0}, .extent = contextPtr->GetSwapchain()->GetExtent() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthAttachment
        };

        commandBuffer.beginRendering(renderInfo);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Handle());

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

        const float ratio = extent.width / (float)extent.height;
        const glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        const glm::mat4 v = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -1.5f)), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
        const glm::mat4 mvp = p * v * m;

        commandBuffer.pushConstants(pipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &mvp);
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
        pipeline.reset();
        depthTexture.reset();

        BaseApp::Cleanup();
    }

    void ModelApp::OnResize(int width, int height)
    {
        BaseApp::OnResize(width, height);

        depthTexture.reset();
        CreateDepthBuffer();
    }

    bool ModelApp::LoadModel()
    {
        const aiScene* scene = aiImportFile("rubber_duck/scene.gltf", aiProcess_Triangulate | aiProcess_MakeLeftHanded);
        if (!scene || !scene->HasMeshes())
        {
            spdlog::error(aiGetErrorString());
            return false;
        }

        const aiMesh* mesh = scene->mMeshes[0];
        
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            const aiVector3D& v = mesh->mVertices[i];
            const aiVector3D& n = mesh->mNormals[i];
            const aiVector3D& t = mesh->mTextureCoords[0][i];
            vertices.push_back({
                .pos = glm::vec3(v.x, v.y, v.z),
                .normal = glm::vec3(n.x, n.y, n.z),
                .uv = glm::vec2(t.x, t.y)
            });
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
                indices.push_back(mesh->mFaces[i].mIndices[j]);
        }

        aiReleaseImport(scene);

        // Vertex Buffer
        std::unique_ptr<VulkanBuffer> stagingVertexBuffer = contextPtr->CreateBuffer(
            sizeof(VertexData) * vertices.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );
        
        vertexBuffer = contextPtr->CreateBuffer(
            sizeof(VertexData) * vertices.size(),
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

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

        contextPtr->BeginCommand();
        contextPtr->UploadBuffer(vertices.data(), stagingVertexBuffer.get(), vertexBuffer.get());
        contextPtr->UploadBuffer(indices.data(), stagingIndexBuffer.get(), indexBuffer.get());
        contextPtr->EndCommand();

        return true;
    }

    bool ModelApp::CreateDepthBuffer()
    {
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const TextureDesc desc{
            .usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
            .format = vk::Format::eD32Sfloat,
            .extent = {
                .width = extent.width, .height = extent.height, .depth = 1
            },
            .aspectMask = vk::ImageAspectFlagBits::eDepth
        };

        const VmaAllocationDesc allocDesc{
            .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
            .usage = vma::MemoryUsage::eAutoPreferDevice
        };

        depthTexture = contextPtr->CreateTexture(desc, allocDesc);

        if (depthTexture == nullptr)
        {
            return false;
        }

        return true;
    }

    bool ModelApp::CreatePipeline()
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {
            { .binding = 0, .stride = sizeof(VertexData), .inputRate = vk::VertexInputRate::eVertex}
        };

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
            { .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = 0 },
            { .location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(VertexData, normal) },
            { .location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat,    .offset = offsetof(VertexData, uv) }
        };

        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex, .offset = 0, .size = sizeof(glm::mat4) }
        };
        std::vector<vk::Format> colorFormats = { contextPtr->GetSwapchain()->GetFormat() };

        IPipelineBuilder pd;
        pd.SetColorFormats(colorFormats);
        pd.SetDepthFormat(depthTexture->GetFormat());
        pd.SetVertexBindingDescriptions(bindingDescriptions);
        pd.SetVertexAttributeDescriptions(attributeDescriptions);
        pd.SetPushConstantRanges(pushConstantRanges);
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
