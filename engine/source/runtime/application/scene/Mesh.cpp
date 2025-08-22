#include "Mesh.h"
#include "ScopeExit.h"

#include <meshoptimizer.h>

namespace jgw
{
    bool IsMeshDataValid(const char* fileName)
    {
        FILE* f = fopen(fileName, "rb");

        if (!f)
            return false;

        SCOPE_EXIT
        {
            fclose(f);
        };

        MeshFileHeader header;

        if (fread(&header, 1, sizeof(header), f) != sizeof(header))
        {
            fclose(f);
            return false;
        }

        return true;
    }

    MeshFileHeader LoadMeshData(const char* fileName, MeshData& out)
    {
        FILE* f = fopen(fileName, "rb");

        if (!f)
        {
            spdlog::error("Cannot open {}.\n", fileName);
            exit(EXIT_FAILURE);
        }

        SCOPE_EXIT
        {
            fclose(f);
        };

        MeshFileHeader header;

        if (fread(&header, 1, sizeof(header), f) != sizeof(header))
        {
            spdlog::error("Unable to read mesh file header.\n");
            exit(EXIT_FAILURE);
        }

        if (fread(&out.streams, 1, sizeof(out.streams), f) != sizeof(out.streams))
        {
            spdlog::error("Could not read vertex streams description.\n");
            exit(EXIT_FAILURE);
        }

        out.meshes.resize(header.meshCount);
        if (fread(out.meshes.data(), sizeof(Mesh), header.meshCount, f) != header.meshCount)
        {
            spdlog::error("Could not read mesh descriptors.\n");
            exit(EXIT_FAILURE);
        }

        out.indexData.resize(header.indexDataSize / sizeof(uint32_t));
        out.vertexData.resize(header.vertexDataSize);

        if (fread(out.indexData.data(), 1, header.indexDataSize, f) != header.indexDataSize)
        {
            spdlog::error("Could not read index data.\n");
            exit(EXIT_FAILURE);
        }

        if (fread(out.vertexData.data(), 1, header.vertexDataSize, f) != header.vertexDataSize)
        {
            spdlog::error("Could not read vertex data.\n");
            exit(EXIT_FAILURE);
        }

        return header;
    }

    void SaveMeshData(const char* fileName, const MeshData& meshData)
    {
        FILE* f = fopen(fileName, "wb");

        if (!f)
        {
            spdlog::error("Error opening file {} for writing.\n", fileName);
            exit(EXIT_FAILURE);
        }

        SCOPE_EXIT
        {
            fclose(f);
        };

        const MeshFileHeader header =
        {
            .meshCount = static_cast<uint32_t>(meshData.meshes.size()),
            .indexDataSize = static_cast<uint32_t>(meshData.indexData.size() * sizeof(uint32_t)),
            .vertexDataSize = static_cast<uint32_t>(meshData.vertexData.size())
        };

        fwrite(&header, 1, sizeof(header), f);
        fwrite(&meshData.streams, 1, sizeof(meshData.streams), f);
        fwrite(meshData.meshes.data(), sizeof(Mesh), header.meshCount, f);
        fwrite(meshData.indexData.data(), 1, header.indexDataSize, f);
        fwrite(meshData.vertexData.data(), 1, header.vertexDataSize, f);
    }

    void LoadMeshFile(const char* fileName, MeshData& meshData)
    {
        const unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_LimitBoneWeights | aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality |
            aiProcess_RemoveRedundantMaterials | aiProcess_FindDegenerates | aiProcess_FindInvalidData |
            aiProcess_GenUVCoords;

        const aiScene* scene = aiImportFile(fileName, flags);
        if (!scene || !scene->HasMeshes())
        {
            spdlog::error(aiGetErrorString());
            exit(EXIT_FAILURE);
        }

        meshData.meshes.reserve(scene->mNumMeshes);

        uint32_t indexOffset = 0;
        uint32_t vertexOffset = 0;

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            fflush(stdout);
            meshData.meshes.push_back(ConvertAIMesh(scene->mMeshes[i], meshData, indexOffset, vertexOffset, true));
        }
    }

    void ProcessLOD(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods)
    {
        const size_t verticesCountIn = vertices.size() / 3;
        size_t targetIndicesCount = indices.size();
        uint8_t LOD = 1;
        outLods.push_back(indices);

        while (targetIndicesCount > 1024 && LOD < kMaxLODs)
        {
            targetIndicesCount = indices.size() / 2;
            bool sloppy = false;

            size_t numOptIndices = meshopt_simplify(
                indices.data(), indices.data(), indices.size(), vertices.data(), verticesCountIn, sizeof(float) * 3, targetIndicesCount, 0.02f
            );

            // Cannot simplify further
            if (static_cast<size_t>(numOptIndices * 1.1f) > indices.size())
            {
                if (LOD > 1)
                {
                    // Try harder
                    numOptIndices = meshopt_simplifySloppy(
                        indices.data(), indices.data(), indices.size(), vertices.data(), verticesCountIn, sizeof(float) * 3, targetIndicesCount, 0.02f
                    );
                    sloppy = true;
                    if (numOptIndices == indices.size())
                        break;
                }
                else break;
            }

            indices.resize(numOptIndices);

            meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), verticesCountIn);

            ++LOD;
            outLods.push_back(indices);
        }
    }

    Mesh ConvertAIMesh(const aiMesh* m, MeshData& meshData, uint32_t& indexOffset, uint32_t& vertexOffset, bool calculateLOD)
    {
        const bool hasTexCoords = m->HasTextureCoords(0);

        // Original data for LOD calculation
        std::vector<float> srcVertices;
        std::vector<uint32_t> srcIndices;
        std::vector<std::vector<uint32_t>> outLods;
        std::vector<uint8_t>& vertices = meshData.vertexData;

        for (size_t i = 0; i < m->mNumVertices; ++i)
        {
            const aiVector3D v = m->mVertices[i];
            const aiVector3D n = m->mNormals[i];
            const aiVector2D t = hasTexCoords ? aiVector2D(m->mTextureCoords[0][i].x, m->mTextureCoords[0][i].y) : aiVector2D();

            if (calculateLOD)
            {
                srcVertices.push_back(v.x);
                srcVertices.push_back(v.y);
                srcVertices.push_back(v.z);
            }

            Put(vertices, v);                                                   // pos   : vec3
            Put(vertices, glm::packHalf2x16(glm::vec2(t.x, t.y)));              // uv    : half2
            Put(vertices, glm::packSnorm3x10_1x2(glm::vec4(n.x, n.y, n.z, 0))); // normal: 2_10_10_10_REV
        }

        // pos, uv, normal
        meshData.streams = {
            .attributes = {
                { .location = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = 0 },                                           // pos
                { .location = 1, .format = vk::Format::eR16G16Sfloat, .offset = sizeof(glm::vec3) },                              // uv
                { .location = 2, .format = vk::Format::eA2B10G10R10SnormPack32, .offset = sizeof(glm::vec3) + sizeof(uint32_t) }  // normal
            },
            .inputBindings = {
                { .stride = sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t) }
            }
        };

        for (unsigned int i = 0; i < m->mNumFaces; ++i)
        {
            if (m->mFaces[i].mNumIndices != 3)
                continue;

            for (unsigned int j = 0; j < m->mFaces[i].mNumIndices; ++j)
                srcIndices.push_back(m->mFaces[i].mIndices[j]);
        }

        if (!calculateLOD)
            outLods.push_back(srcIndices);
        else
            ProcessLOD(srcIndices, srcVertices, outLods);

        Mesh result =
        {
            .indexOffset = indexOffset,
            .vertexOffset = vertexOffset,
            .vertexCount = m->mNumVertices
        };

        uint32_t numIndices = 0;
        for (size_t l = 0; l < outLods.size(); ++l)
        {
            for (size_t i = 0; i < outLods[l].size(); ++i)
                meshData.indexData.push_back(outLods[l][i]);

            result.lodOffset[l] = numIndices;
            numIndices += outLods[l].size();
        }

        result.lodOffset[outLods.size()] = numIndices;
        result.lodCount = outLods.size();

        indexOffset += numIndices;
        vertexOffset += m->mNumVertices;

        return result;
    }
}
