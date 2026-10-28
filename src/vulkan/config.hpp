#pragma once

#include "vulkan/vk.hpp"

#include <vector>

#ifdef NDEBUG
constexpr bool DEBUG = false;
#else
constexpr bool DEBUG = true;
#endif

inline const std::vector<char const*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
inline const std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};
