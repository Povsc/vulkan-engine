#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vk.hpp"

class HelloTriangleApplication {
  public:
    void run();

  private:
    // TODO: reuse Window;Camera;Helper classes from my OpenGL project
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    GLFWwindow* window_ = nullptr;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::raii::PhysicalDevice physicalDevice_ = nullptr;
};
