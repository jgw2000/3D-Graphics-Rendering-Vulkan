#pragma once

#include "Macro.h"

#include <vulkan/vulkan.hpp>
#include <vector>

namespace jgw
{
    class RenderContext final
    {
    public:
        CLASS_COPY_MOVE_DELETE(RenderContext)

	    RenderContext() = default;
        ~RenderContext();

        bool Initialize(
            const std::vector<const char*>& requestInstanceLayers,
            const std::vector<const char*>& requestInstanceExtensions,
			uint32_t apiVersion = VK_API_VERSION_1_3
        );

    private:
		bool CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const;
		bool CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const;

    private:
        vk::Instance instance = nullptr;
    };
}
