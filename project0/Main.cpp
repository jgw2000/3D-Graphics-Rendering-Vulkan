#include "Project0.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 0";

    jgw::Project0 app(config);
    app.Start();

    return 0;
}
