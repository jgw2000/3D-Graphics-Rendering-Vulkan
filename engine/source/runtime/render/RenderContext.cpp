#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "RenderContext.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define FMT_UNICODE 0
#include <spdlog/spdlog.h>

namespace jgw
{
	RenderContext::~RenderContext()
	{
		if (instance)
		{
			instance.destroy();
			instance = nullptr;
		}
	}

	bool RenderContext::Initialize(
		const std::vector<const char*>& requestInstanceLayers,
		const std::vector<const char*>& requestInstanceExtensions,
		uint32_t apiVersion
	)
	{
		// initialize minimal set of function pointers
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		if (!CheckInstanceLayerSupport(requestInstanceLayers) || !CheckInstanceExtensionSupport(requestInstanceExtensions))
		{
			return false;
		}

		vk::ApplicationInfo appInfo("Vulkan Project", VK_MAKE_VERSION(1, 0, 0), "Vulkan", {}, apiVersion);
		vk::InstanceCreateInfo instanceInfo({}, &appInfo, requestInstanceLayers, requestInstanceExtensions);
		vk::Instance instance = vk::createInstance(instanceInfo);

		return true;
	}

	bool RenderContext::CheckInstanceLayerSupport(const std::vector<const char*>& requestInstanceLayers) const
	{
		if (requestInstanceLayers.empty())
			return true;

		auto availableLayers = vk::enumerateInstanceLayerProperties();
		for (const auto& layer : requestInstanceLayers)
		{
			bool found = false;
			for (const auto& availableLayer : availableLayers)
			{
				if (strcmp(layer, availableLayer.layerName.data()) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				spdlog::error("Instance layer {} not found", layer);
				return false;
			}
		}

		return true;
	}

	bool RenderContext::CheckInstanceExtensionSupport(const std::vector<const char*>& requestInstanceExtensions) const
	{
		if (requestInstanceExtensions.empty())
			return true;

		auto availableExtensions = vk::enumerateInstanceExtensionProperties();
		for (const auto& extension : requestInstanceExtensions)
		{
			bool found = false;
			for (const auto& availableExtension : availableExtensions)
			{
				if (strcmp(extension, availableExtension.extensionName.data()) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				spdlog::error("Instance extension {} not found", extension);
				return false;
			}
		}
		return true;
	}
}
