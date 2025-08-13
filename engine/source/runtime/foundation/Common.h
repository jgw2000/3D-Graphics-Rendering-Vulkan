#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <spdlog/spdlog.h>

#include "VmaUsage.h"
#include "Macro.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
