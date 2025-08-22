#include "Project2.h"

#include <meshoptimizer.h>
#include <glm/gtc/type_ptr.hpp>

namespace jgw
{
    Project2::Project2(const WindowConfig& config) : BaseApp(config)
    {
        contextPtr->GetDeviceFeatures().geometryShader = vk::True;
        contextPtr->GetDeviceFeatures().tessellationShader = vk::True;
    }

    bool Project2::OnInit()
    {
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

        SetupCamera();

        return true;
    }

    void Project2::OnUpdate(double delta)
    {
        cameraPtr->Update(delta);

        pcData.view = cameraPtr->GetViewMatrix();
        pcData.proj = cameraPtr->GetProjMatrix();
        pcData.cameraPos = glm::vec4(cameraPtr->GetPosition(), 1);

        canvasGrid->SetMatrix(pcData.proj * pcData.view);
        canvasGrid->SetCameraPos(glm::vec4(cameraPtr->GetPosition(), 1));
    }

    void Project2::OnRender(vk::CommandBuffer commandBuffer)
    {
        vk::ClearValue colorCV{
            .color = std::array<float, 4>({1.0f, 1.0f, 1.0f, 1.0f}),
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

        pcData.model = glm::rotate(glm::mat4(1.0f), -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        commandBuffer.pushConstants(pipeline->Layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl, 0, sizeof(PushConstantData), &pcData);
        commandBuffer.drawIndexed(indices.size(), 100, 0, 0, 0);

        canvas3D->Render(*contextPtr);
        canvasGrid->Render(*contextPtr);

        imguiPtr->Render(commandBuffer);

        commandBuffer.endRendering();
    }

    void Project2::OnCleanup()
    {
        vertexBuffer.reset();
        indexBuffer.reset();
        pipeline.reset();
    }

    void Project2::OnResize(int width, int height)
    {
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetAspectRatio(aspect);
    }

    bool Project2::LoadModel()
    {
        const aiScene* scene = aiImportFile("../assets/rubber_duck/scene.gltf", aiProcess_Triangulate);
        if (!scene || !scene->HasMeshes())
        {
            spdlog::error(aiGetErrorString());
            return false;
        }

        const aiMesh* mesh = scene->mMeshes[0];

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            const aiVector3D& v = mesh->mVertices[i];
            vertices.push_back(glm::vec3(v.x, v.y, v.z));
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
                indices.push_back(mesh->mFaces[i].mIndices[j]);
        }

        aiReleaseImport(scene);

        OptimizeMesh();

        std::unique_ptr<VulkanBuffer> stagingVertexBuffer = contextPtr->CreateBuffer(
            sizeof(glm::vec3) * vertices.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        vertexBuffer = contextPtr->CreateBuffer(
            sizeof(glm::vec3) * vertices.size(),
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

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

    bool Project2::CreatePipeline()
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {
            { .binding = 0, .stride = sizeof(glm::vec3), .inputRate = vk::VertexInputRate::eVertex }
        };

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
            { .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = 0 }
        };

        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl, .offset = 0, .size = sizeof(PushConstantData) }
        };

        PipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "shaders/main.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eTessellationControl, "shaders/main.tesc.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eTessellationEvaluation, "shaders/main.tese.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eGeometry, "shaders/main.geom.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "shaders/main.frag.spv");
        pd.InputAssemblyCI().topology = vk::PrimitiveTopology::ePatchList;
        pd.TessellationStateCI().patchControlPoints = 3;
        pd.SetVertexBindingDescriptions(bindingDescriptions);
        pd.SetVertexAttributeDescriptions(attributeDescriptions);
        pd.SetPushConstantRanges(pushConstantRanges);

        pipeline = contextPtr->CreateGraphicsPipeline(pd);
        if (pipeline == nullptr)
        {
            return false;
        }

        return true;
    }

    void Project2::SetupCamera()
    {
        cameraPtr->SetPosition(glm::vec3(0.5f, 1.0f, 3.0f));
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }

    void Project2::OptimizeMesh()
    {
        std::vector<uint32_t> remap(indices.size());
        const size_t vertexCount = meshopt_generateVertexRemap(
            remap.data(), indices.data(), indices.size(), vertices.data(), vertices.size(), sizeof(glm::vec3)
        );

        std::vector<uint32_t> remappedIndices(indices.size());
        std::vector<glm::vec3> remappedVertices(vertexCount);

        meshopt_remapIndexBuffer(remappedIndices.data(), indices.data(), indices.size(), remap.data());
        meshopt_remapVertexBuffer(remappedVertices.data(), vertices.data(), vertices.size(), sizeof(glm::vec3), remap.data());

        meshopt_optimizeVertexCache(remappedIndices.data(), remappedIndices.data(), indices.size(), vertexCount);
        meshopt_optimizeOverdraw(remappedIndices.data(), remappedIndices.data(), indices.size(), glm::value_ptr(remappedVertices[0]), vertexCount, sizeof(glm::vec3), 1.05f);
        meshopt_optimizeVertexFetch(remappedVertices.data(), remappedIndices.data(), indices.size(), remappedVertices.data(), vertexCount, sizeof(glm::vec3));

        const float threshold = 0.2f;
        const size_t targetIndexCount = size_t(remappedIndices.size() * threshold);
        const float targetError = 1e-2f;

        indicesLod.resize(remappedIndices.size());
        indicesLod.resize(meshopt_simplify(
            &indicesLod[0], remappedIndices.data(), remappedIndices.size(), &remappedVertices[0].x, vertexCount, sizeof(glm::vec3),
            targetIndexCount, targetError
        ));

        indices = remappedIndices;
        vertices = remappedVertices;
    }
}
