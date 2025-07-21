#pragma once

#include "Macro.h"
#include "Window.h"
#include "RenderContext.h"

#include <memory>
#include <vector>

namespace jgw
{
    class BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(BaseApp)

        BaseApp();
        BaseApp(const WindowConfig& config);

        void Run();

    protected:
        virtual void OnKey(int key, int scancode, int action, int mods) {}

        virtual std::vector<const char*> GetInstanceLayers() const;
        virtual std::vector<const char*> GetInstanceExtensions() const;

    private:
        bool Initialize();

        std::unique_ptr<Window> windowPtr;
        std::unique_ptr<RenderContext> contextPtr;
    };
}
