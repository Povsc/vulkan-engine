#pragma once

#include "glfw.hpp"
#include "vulkan/vk.hpp"

vk::raii::SurfaceKHR createSurface(vk::raii::Instance& instance, GLFWwindow* window);