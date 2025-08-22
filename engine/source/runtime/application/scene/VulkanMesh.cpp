#include "VulkanMesh.h"

namespace jgw
{
    VulkanMesh::VulkanMesh(VulkanContext& context, const MeshFileHeader& header, const MeshData& meshData)
        : numIndices(header.indexDataSize / sizeof(uint32_t))
        , header(header)
    {
        const uint32_t* indices = meshData.indexData.data();
        const uint8_t* vertexData = meshData.vertexData.data();

        std::unique_ptr<VulkanBuffer> stagingVertexBuffer = context.CreateBuffer(
            header.vertexDataSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        vertexBuffer = context.CreateBuffer(
            header.vertexDataSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

        std::unique_ptr<VulkanBuffer> stagingIndexBuffer = context.CreateBuffer(
            header.indexDataSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        indexBuffer = context.CreateBuffer(
            header.indexDataSize,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

        const uint32_t numCommands = header.meshCount;
        std::vector<uint8_t> drawCommands(sizeof(DrawIndexedIndirectCommand) * numCommands + sizeof(uint32_t));

        // Store the number of draw commands in the very beginning of the buffer
        memcpy(drawCommands.data(), &numCommands, sizeof(numCommands));

        DrawIndexedIndirectCommand* cmd = std::launder(reinterpret_cast<DrawIndexedIndirectCommand*>(drawCommands.data() + sizeof(uint32_t)));

        // Prepare indirect command buffer
        for (uint32_t i = 0; i < numCommands; ++i)
        {
            *cmd++ = {
                .count         = meshData.meshes[i].GetLODIndicesCount(0),
                .instanceCount = 1,
                .firstIndex    = meshData.meshes[i].indexOffset,
                .baseVertex    = (int32_t)meshData.meshes[i].vertexOffset,
                .baseInstance  = 0
            };
        }

        std::unique_ptr<VulkanBuffer> stagingIndirectBuffer = context.CreateBuffer(
            sizeof(DrawIndexedIndirectCommand) * numCommands + sizeof(uint32_t),
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
        );

        indirectBuffer = context.CreateBuffer(
            sizeof(DrawIndexedIndirectCommand) * numCommands + sizeof(uint32_t),
            vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst
        );

        context.BeginCommand();
        context.UploadBuffer(vertexData, stagingVertexBuffer.get(), vertexBuffer.get());
        context.UploadBuffer(indices, stagingIndexBuffer.get(), indexBuffer.get());
        context.UploadBuffer(drawCommands.data(), stagingIndirectBuffer.get(), indirectBuffer.get());
        context.EndCommand();

        CreatePipeline(context, header, meshData);
    }

    void VulkanMesh::Draw(vk::CommandBuffer commandBuffer)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Handle());

        vk::Buffer vertexBuffers[] = { vertexBuffer->Handle() };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindIndexBuffer(indexBuffer->Handle(), 0, vk::IndexType::eUint32);
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.pushConstants(pipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData), &pcData);
        commandBuffer.drawIndexedIndirect(indirectBuffer->Handle(), sizeof(uint32_t), header.meshCount, sizeof(DrawIndexedIndirectCommand));
    }

    void VulkanMesh::CreatePipeline(VulkanContext& context, const MeshFileHeader& header, const MeshData& meshData)
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
        for (uint32_t i = 0; i < meshData.streams.GetInputBindingNum(); ++i)
        {
            const auto& bd = meshData.streams.inputBindings[i];
            bindingDescriptions.push_back({
                .binding = i, .stride = bd.stride, .inputRate = vk::VertexInputRate::eVertex
            });
        }

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        for (uint32_t i = 0; i < meshData.streams.GetAttributeNum(); ++i)
        {
            const auto& attr = meshData.streams.attributes[i];
            attributeDescriptions.push_back({
                .location = i, .binding = attr.binding, .format = attr.format, .offset = static_cast<uint32_t>(attr.offset)
            });
        }

        std::vector<vk::PushConstantRange> pushConstantRanges = {
            { .stageFlags = vk::ShaderStageFlagBits::eVertex, .offset = 0, .size = sizeof(PushConstantData) }
        };

        PipelineBuilder pd;
        pd.AddShader(vk::ShaderStageFlagBits::eVertex, "../engine/shaders/mesh.vert.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eGeometry, "../engine/shaders/mesh.geom.spv");
        pd.AddShader(vk::ShaderStageFlagBits::eFragment, "../engine/shaders/mesh.frag.spv");
        pd.SetVertexBindingDescriptions(bindingDescriptions);
        pd.SetVertexAttributeDescriptions(attributeDescriptions);
        pd.SetPushConstantRanges(pushConstantRanges);

        pipeline = context.CreateGraphicsPipeline(pd);
        if (pipeline == nullptr)
        {
            spdlog::error("VulkanMesh create pipeline failed\n");
            exit(EXIT_FAILURE);
        }
    }
}