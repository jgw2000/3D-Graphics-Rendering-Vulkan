#include "ModelApp.h"

#include <spdlog/spdlog.h>
#include <vector>

namespace jgw
{
    ModelApp::ModelApp(const WindowConfig& config) : BaseApp(config)
    {

    }

    bool ModelApp::Initialize()
    {
        if (!BaseApp::Initialize())
            return false;

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

        return true;
    }

    void ModelApp::Render(vk::CommandBuffer commandBuffer)
    {
        
    }

    bool ModelApp::LoadModel()
    {
        const aiScene* scene = aiImportFile("rubber_duck/scene.gltf", aiProcess_Triangulate);
        if (!scene || !scene->HasMeshes())
        {
            spdlog::error(aiGetErrorString());
            return false;
        }

        const aiMesh* mesh = scene->mMeshes[0];

        std::vector<glm::vec3> positions;
        std::vector<uint32_t> indices;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            const aiVector3D& vertex = mesh->mVertices[i];
            positions.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
                indices.push_back(mesh->mFaces[i].mIndices[j]);
        }

        aiReleaseImport(scene);

        return true;
    }

    bool ModelApp::CreatePipeline()
    {
        return true;
    }
}
