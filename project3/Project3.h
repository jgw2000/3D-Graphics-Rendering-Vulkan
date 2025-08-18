#pragma once

#include "BaseApp.h"

namespace jgw
{
    class Project3 : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(Project3)

        Project3(const WindowConfig& config = {});

    protected:
        virtual bool OnInit() override;
        virtual void OnUpdate(double delta) override;
        virtual void OnRender(vk::CommandBuffer commandBuffer) override;
        virtual void OnCleanup() override;
        virtual void OnResize(int width, int height) override;

    private:
        bool LoadModel();
        bool CreatePipeline();
        void SetupCamera();
        void OptimizeMesh();

        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> indicesLod;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        std::unique_ptr<VulkanBuffer> indexLodBuffer;
        std::unique_ptr<VulkanPipeline> pipeline;

        struct PushConstantData
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        } pcData;
    };
}
