#pragma once

#include "Vertex.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace jgw
{
    // All offsets are relative to the beginning of the data block (excluding headers with a Mesh list)
    struct Mesh final
    {
        // Number of LODs in this mesh
        uint32_t lodCount = 1;

        // The total count of all previous vertices in this mesh file
        uint32_t indexOffset = 0;

        uint32_t vertexOffset = 0;

        // Vertex count (for all LODs)
        uint32_t vertexCount = 0;

        // Offsets to LOD indices data. The last offset is used as a marker to calculate the size
        uint32_t lodOffset[kMaxLODs + 1] = { 0 };

        uint32_t materialID = 0;

        inline uint32_t GetLODIndicesCount(uint32_t lod) const
        {
            return lod < lodCount ? lodOffset[lod + 1] - lodOffset[lod] : 0;
        }
    };

    struct MeshFileHeader
    {
        // Unique 32-bit value to check integrity of the file
        uint32_t magicValue = 0x12345678;

        // Number of mesh descriptors following this header
        uint32_t meshCount = 0;

        // How much space index data takes in bytes
        uint32_t indexDataSize = 0;

        // How mush space vertex data takes in bytes
        uint32_t vertexDataSize = 0;
    };

    struct MeshData
    {
        VertexInput streams = {};
        std::vector<uint32_t> indexData;
        std::vector<uint8_t> vertexData;
        std::vector<Mesh> meshes;

        MeshFileHeader GetMeshFileHeader() const
        {
            return {
                .meshCount = static_cast<uint32_t>(meshes.size()),
                .indexDataSize = static_cast<uint32_t>(indexData.size() * sizeof(uint32_t)),
                .vertexDataSize = static_cast<uint32_t>(vertexData.size()),
            };
        }
    };

    bool IsMeshDataValid(const char* fileName);

    MeshFileHeader LoadMeshData(const char* fileName, MeshData& out);

    void SaveMeshData(const char* fileName, const MeshData& meshData);

    void LoadMeshFile(const char* fileName, MeshData& meshData);

    void ProcessLOD(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods);

    Mesh ConvertAIMesh(const aiMesh* m, MeshData& meshData, uint32_t& indexOffset, uint32_t& vertexOffset, bool calculateLOD = false);
}
