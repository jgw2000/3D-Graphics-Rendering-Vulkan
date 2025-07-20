#include "BaseApp.h"

int main()
{
    jgw::WindowConfig config;
    config.title = "Vulkan";

    jgw::BaseApp app(config);
    app.Run();

    return 0;
}
