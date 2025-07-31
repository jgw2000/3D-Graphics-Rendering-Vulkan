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
        virtual bool Initialize() override;
        virtual void Render(vk::CommandBuffer commandBuffer) override;
        virtual void Cleanup() override;
        virtual void OnResize(int width, int height) override;

    private:
        bool LoadModel();
        bool CreateDepthBuffer();
        bool CreatePipeline();

        std::vector<VertexData> vertices;
        std::vector<uint32_t> indices;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanTexture> depthTexture;
        std::unique_ptr<VulkanTexture> modelTexture;
    };
}
