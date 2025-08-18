#include "Project2.h"

namespace jgw
{
    const glm::vec3 kInitialCameraPos = glm::vec3(0.0f, 1.0f, -1.5f);
    const glm::vec3 kInitialCameraTarget = glm::vec3(0.0f, 0.5f, 0.0f);

    Project2::Project2(const WindowConfig& config) : BaseApp(config)
    {
        pcData.model = glm::rotate(glm::mat4(1.0f), -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    bool Project2::OnInit()
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

        SetupCamera();

        return true;
    }

    void Project2::OnUpdate(double delta)
    {
        cameraPtr->Update(delta);
    }

    void Project2::OnRender(vk::CommandBuffer commandBuffer)
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
        pcData.view = cameraPtr->GetViewMatrix();
        pcData.proj = cameraPtr->GetProjMatrix();
        pcData.cameraPos = glm::vec4(cameraPtr->GetPosition(), 1);

        commandBuffer.pushConstants(pipeline->Layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData), &pcData);
        commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

        // Render skybox
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxPipeline->Handle());
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxPipeline->Layout(), 0, 1, &descriptorSet, 0, nullptr);
        commandBuffer.pushConstants(skyboxPipeline->Layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData), &pcData);
        commandBuffer.draw(36, 1, 0, 0);

        canvas3D->Render(*contextPtr);
        imguiPtr->Render(commandBuffer);

        commandBuffer.endRendering();
    }

    void Project2::OnCleanup()
    {
        auto device = GetDevice();
        device.destroyDescriptorSetLayout(descriptorSetLayout);
        device.destroyDescriptorPool(descriptorPool);
        device.destroySampler(sampler);

        vertexBuffer.reset();
        indexBuffer.reset();
        pipeline.reset();
        skyboxPipeline.reset();
        modelTexture.reset();
        cubeTexture.reset();
    }

    void Project2::OnResize(int width, int height)
    {
        BaseApp::OnResize(width, height);

        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetAspectRatio(aspect);
    }

    void Project2::OnGizmos()
    {
        auto viewMatrix = cameraPtr->GetViewMatrix();
        auto projMatrix = cameraPtr->GetProjMatrix();

        canvas3D->SetMatrix(projMatrix * viewMatrix);
        canvas3D->Plane(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 40, 40, 10.0f, 10.0f, glm::vec4(1, 0, 0, 1), glm::vec4(0, 1, 0, 1));
        canvas3D->Box(pcData.model, glm::vec3(2, 2, 2), glm::vec4(1, 1, 0, 1));
        canvas3D->Frustum(
            glm::lookAt(glm::vec3(cos(glfwGetTime()), kInitialCameraPos.y, sin(glfwGetTime())), kInitialCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f)),
            projMatrix, glm::vec4(1, 1, 1, 1)
        );
    }

    bool Project2::LoadModel()
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
        cubeTexture = LoadCubeTexture("data/cubemap_yokohama_rgba.ktx", vk::Format::eR8G8B8A8Unorm);

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

    bool Project2::CreateDescriptors()
    {
        auto device = GetDevice();

        // Create descriptor set layout
        std::vector<vk::DescriptorSetLayoutBinding> samplerBindings = {
            {
                .binding = 0,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment
            },
            {
                .binding = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment
            }
        };

        vk::DescriptorSetLayoutCreateInfo layoutCI{
            .bindingCount = static_cast<uint32_t>(samplerBindings.size()),
            .pBindings = samplerBindings.data()
        };

        descriptorSetLayout = device.createDescriptorSetLayout(layoutCI);

        // Create descriptor pool
        vk::DescriptorPoolSize poolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 2
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

        imageInfo.imageView = cubeTexture->GetView();
        descriptorWrite.dstBinding = 1;
        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        return true;
    }

    bool Project2::CreatePipeline()
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
            { .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, .offset = 0, .size = sizeof(PushConstantData) }
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

        PipelineBuilder skyboxPd;
        skyboxPd.AddShader(vk::ShaderStageFlagBits::eVertex, "shaders/skybox.vert.spv");
        skyboxPd.AddShader(vk::ShaderStageFlagBits::eFragment, "shaders/skybox.frag.spv");
        skyboxPd.SetDescriptorSetLayouts(descriptorSetLayouts);
        skyboxPd.SetPushConstantRanges(pushConstantRanges);

        skyboxPipeline = contextPtr->CreateGraphicsPipeline(skyboxPd);
        if (skyboxPipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void Project2::SetupCamera()
    {
        cameraPtr->SetPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }
}
