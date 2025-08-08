#pragma once

#include "BaseApp.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

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
        virtual void Cleanup() override;
        virtual void OnResize(int width, int height) override;
        virtual void OnKey(int key, int scancode, int action, int mods) override;
        virtual void OnMouse(int button, int action, int mods) override;

    private:
        bool LoadModel();
        bool CreateDescriptors();
        bool CreatePipeline();

        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanTexture> depthTexture;
        std::unique_ptr<VulkanTexture> modelTexture;

        vk::Sampler sampler{};
        vk::DescriptorPool descriptorPool{};
        vk::DescriptorSet descriptorSet{};
        vk::DescriptorSetLayout descriptorSetLayout{};
    };
}
