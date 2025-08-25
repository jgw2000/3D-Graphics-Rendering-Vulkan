#pragma once

#include "BaseApp.h"

namespace jgw
{
    class Project4 : public BaseApp
    {
    public:
        CLASS_COPY_MOVE_DELETE(Project4)

        Project4(const WindowConfig& config = {});
    };
}