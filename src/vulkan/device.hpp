#pragma once

#include "vulkan/vk.hpp"

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance& instance);
vk::raii::Device createLogicalDevice(vk::raii::PhysicalDevice& physicalDevice, uint32_t& graphicsFamilyIndex);