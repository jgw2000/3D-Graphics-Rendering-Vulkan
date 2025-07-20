#pragma once

#include "Macro.h"
#include "Window.h"

#include <memory>

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
        virtual bool OnInit() { return true; }
        virtual void OnKey(int key, int scancode, int action, int mods) {}

    private:
        bool Initialize();

        std::unique_ptr<Window> windowPtr;
    };
}
