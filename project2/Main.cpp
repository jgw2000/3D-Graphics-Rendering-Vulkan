#include "ModelApp.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 2 - Model";

    jgw::ModelApp app(config);
    app.Start();

    return 0;
}