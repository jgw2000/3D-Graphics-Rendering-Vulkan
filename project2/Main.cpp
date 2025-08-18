#include "Project2.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 2";

    jgw::Project2 app(config);
    app.Start();

    return 0;
}