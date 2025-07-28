#include "ModelApp.h"

namespace jgw
{
    ModelApp::ModelApp(const WindowConfig& config) : BaseApp(config)
    {

    }

    bool ModelApp::Initialize()
    {
        if (!BaseApp::Initialize())
            return false;

        return true;
    }

    void ModelApp::Render(vk::CommandBuffer commandBuffer)
    {
        
    }
}