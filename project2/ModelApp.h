#pragma once

#include "BaseApp.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace jgw
{
    struct VertexData
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    class ModelApp : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(ModelApp)

        ModelApp(const WindowConfig& config = {});

    protected:
        virtual bool OnInit() override;
        virtual void OnUpdate(double delta) override;
        virtual void OnRender(vk::CommandBuffer commandBuffer) override;
        virtual void OnCleanup() override;
        virtual void OnResize(int width, int height) override;
        virtual void OnGizmos() override;

    private:
        bool LoadModel();
        bool CreateDescriptors();
        bool CreatePipeline();
        void SetupCamera();

        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanTexture> modelTexture;

        vk::Sampler sampler{};
        vk::DescriptorPool descriptorPool{};
        vk::DescriptorSet descriptorSet{};
        vk::DescriptorSetLayout descriptorSetLayout{};
    };
}
