#include "TriangleApp.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan";

    jgw::TriangleApp app(config);
    app.Start();

    return 0;
}
