#pragma once

#include "vulkan/vk.hpp"

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance& instance);
