#pragma once

#include "vulkan/vk.hpp"

vk::raii::Instance createInstance(vk::raii::Context& context);
vk::raii::DebugUtilsMessengerEXT setupDebugMessenger(vk::raii::Instance& instance);
