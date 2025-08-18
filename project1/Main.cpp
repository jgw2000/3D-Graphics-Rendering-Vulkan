#include "Project1.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 1 - Triangle";

    jgw::Project1 app(config);
    app.Start();

    return 0;
}
