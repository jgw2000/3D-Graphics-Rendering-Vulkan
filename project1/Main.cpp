#include "TriangleApp.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan Project 1 - Triangle";

    jgw::TriangleApp app(config);
    app.Start();

    return 0;
}
