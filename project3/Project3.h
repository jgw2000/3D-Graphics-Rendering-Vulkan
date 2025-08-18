#pragma once

#include "BaseApp.h"

namespace jgw
{
    class Project3 : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(Project3)

        Project3(const WindowConfig& config = {});
    };
}
