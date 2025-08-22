#pragma once

#include "Mesh.h"
#include "VulkanContext.h"

namespace jgw
{
    class VulkanMesh final
    {
    public:
        CLASS_COPY_MOVE_DELETE(VulkanMesh)

        VulkanMesh(VulkanContext& context, const MeshFileHeader& header, const MeshData& meshData);

        inline void SetMVP(glm::mat4 m) { pcData.mvp = m; }
        void Draw(vk::CommandBuffer commandBuffer);

    private:
        void CreatePipeline(VulkanContext& context, const MeshFileHeader& header, const MeshData& meshData);

        struct DrawIndexedIndirectCommand
        {
            uint32_t count;
            uint32_t instanceCount;
            uint32_t firstIndex;
            int32_t  baseVertex;
            uint32_t baseInstance;
        };

        struct PushConstantData
        {
            glm::mat4 mvp;
        } pcData;

        uint32_t numIndices;
        MeshFileHeader header;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        std::unique_ptr<VulkanBuffer> indirectBuffer;
        std::unique_ptr<VulkanPipeline> pipeline;
    };
}