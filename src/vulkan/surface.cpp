#include "vulkan/surface.hpp"

vk::raii::SurfaceKHR createSurface(vk::raii::Instance& instance, GLFWwindow* window) {
    VkSurfaceKHR c_surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &c_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    return  vk::raii::SurfaceKHR(instance, c_surface);
}