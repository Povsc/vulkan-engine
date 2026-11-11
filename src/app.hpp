#pragma once

#include "glfw.hpp"
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
    
    vk::raii::SurfaceKHR surface_ = nullptr;

    vk::raii::PhysicalDevice physicalDevice_ = nullptr;
    vk::raii::Device device_ = nullptr;
    uint32_t graphicsFamilyIndex_ = 0;
    vk::raii::Queue graphicsQueue_ = nullptr;
};
