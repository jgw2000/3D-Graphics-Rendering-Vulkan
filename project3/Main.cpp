#include "Project3.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 3";

    jgw::Project3 app(config);
    app.Start();

    return 0;
}