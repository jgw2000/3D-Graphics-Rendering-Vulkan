#include "Project3.h"
#include "scene/Mesh.h"

namespace jgw
{
    Project3::Project3(const WindowConfig& config) : BaseApp(config)
    {
        contextPtr->GetDeviceFeatures().geometryShader = vk::True;
        contextPtr->GetDeviceFeatures().multiDrawIndirect = vk::True;
    }

    bool Project3::OnInit()
    {
        if (!LoadScene())
        {
            spdlog::error("Failed to load scene");
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

    void Project3::OnUpdate(double delta)
    {
        cameraPtr->Update(delta);

        glm::mat4 mvp = cameraPtr->GetProjMatrix() * cameraPtr->GetViewMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
        scene->SetMVP(mvp);
    }

    void Project3::OnRender(vk::CommandBuffer commandBuffer)
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
            .depthStencil = {.depth = 1.0f }
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

        scene->Draw(commandBuffer);

        imguiPtr->Render(commandBuffer);

        commandBuffer.endRendering();
    }

    void Project3::OnCleanup()
    {
        scene.reset();
    }

    void Project3::OnResize(int width, int height)
    {
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetAspectRatio(aspect);
    }

    bool Project3::LoadScene()
    {
        const char* cacheData = "../cache/bistro.meshes";
        if (!IsMeshDataValid(cacheData))
        {
            spdlog::info("No cached mesh data found. Precaching ... \n\n");
            MeshData meshData;
            LoadMeshFile("../deps/src/bistro/Exterior/exterior.obj", meshData);
            SaveMeshData(cacheData, meshData);
        }

        MeshData meshData;
        const MeshFileHeader header = LoadMeshData(cacheData, meshData);

        scene = std::make_unique<VulkanMesh>(*contextPtr, header, meshData);

        return true;
    }

    bool Project3::CreatePipeline()
    {
        return true;
    }

    void Project3::SetupCamera()
    {
        cameraPtr->SetPosition(glm::vec3(-14.894f, 5.743f, -5.527f));
        auto extent = contextPtr->GetSwapchain()->GetExtent();
        const float aspect = extent.width / (float)extent.height;
        cameraPtr->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }
}