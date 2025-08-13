#include "ModelApp.h"

namespace jgw
{
    ModelApp::ModelApp(const WindowConfig& config) : BaseApp(config)
    {
    }

    bool ModelApp::OnInit()
    {
        if (!LoadModel())
        {
            spdlog::error("Failed to load model");
            return false;
        }

        if (!CreateDescriptors())
        {
            spdlog::error("Failed to create descriptors");
            return false;
        }

        if (!CreatePipeline())
        {
            spdlog::error("Failed to create pipeline");
            return false;
        }

        if (!InitImgui(vk::Format::eD32Sfloat))
        {
            spdlog::error("Failed to initialize imgui");
            return false;
        }

        SetupCamera();

        return true;
    }

    void ModelApp::OnUpdate(double delta)
    {
        cameraPtr->Update(delta);
    }

    void ModelApp::OnRender(vk::CommandBuffer commandBuffer)
    {
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
            .imageView = contextPtr->GetDepthTexture()->GetView(),
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
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->Layout(), 0, 1, &descriptorSet, 0, nullptr);

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
        const glm::mat4 m = glm::rotate(glm::mat4(1.0f), -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 v = cameraPtr->GetViewMatrix();
        const glm::mat4 p = cameraPtr->GetProjMatrix();
        const glm::mat4 mvp = p * v * m;

        commandBuffer.pushConstants(pipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &mvp);
        commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

        imguiPtr->Render(commandBuffer);

        commandBuffer.endRendering();
    }

    void ModelApp::OnCleanup()
    {
        auto device = GetDevice();
        device.destroyDescriptorSetLayout(descriptorSetLayout);
        device.destroyDescriptorPool(descriptorPool);
        device.destroySampler(sampler);

        vertexBuffer.reset();
        indexBuffer.reset();
        pipeline.reset();
        modelTexture.reset();
    }

    void ModelApp::OnResize(int width, int height)
    {
        BaseApp::OnResize(width, height);

        SetupCamera();
    }

    void ModelApp::OnGizmos()
    {
        // TODO
        canvasPtr->Line(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec4(1.0f));
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

        modelTexture = LoadTexture("rubber_duck/textures/Duck_baseColor.png", true);

        vk::SamplerCreateInfo samplerCI{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .minLod = 0,
            .maxLod = 13
        };
        sampler = GetDevice().createSampler(samplerCI);
        
        return true;
    }

    bool ModelApp::CreateDescriptors()
    {
        auto device = GetDevice();

        // Create descriptor set layout
        vk::DescriptorSetLayoutBinding samplerBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
        };

        vk::DescriptorSetLayoutCreateInfo layoutCI{
            .bindingCount = 1,
            .pBindings = &samplerBinding
        };

        descriptorSetLayout = device.createDescriptorSetLayout(layoutCI);

        // Create descriptor pool
        vk::DescriptorPoolSize poolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1
        };

        vk::DescriptorPoolCreateInfo poolInfo{
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };

        descriptorPool = device.createDescriptorPool(poolInfo);

        // Create descriptor sets
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout
        };

        descriptorSet = device.allocateDescriptorSets(allocInfo)[0];

        vk::DescriptorImageInfo imageInfo{
            .sampler = sampler,
            .imageView = modelTexture->GetView(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo
        };

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

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

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { descriptorSetLayout };
        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex, .offset = 0, .size = sizeof(glm::mat4) }
        };

        PipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "shaders/model.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "shaders/model.frag.spv");
        pd.SetVertexBindingDescriptions(bindingDescriptions);
        pd.SetVertexAttributeDescriptions(attributeDescriptions);
        pd.SetDescriptorSetLayouts(descriptorSetLayouts);
        pd.SetPushConstantRanges(pushConstantRanges);

        pipeline = contextPtr->CreateGraphicsPipeline(pd);
        if (pipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void ModelApp::SetupCamera()
    {
        cameraPtr->SetPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetPerspective(45.0f, aspect, 0.1f, 1000.0f);
    }
}
