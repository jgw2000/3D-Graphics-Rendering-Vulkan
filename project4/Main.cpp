#include "Project4.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 4";

    jgw::Project4 app(config);
    app.Start();

    return 0;
}