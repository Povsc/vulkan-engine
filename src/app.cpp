#include "app.hpp"

#include "vulkan/device.hpp"
#include "vulkan/instance.hpp"

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;

void HelloTriangleApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulcão", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {
    instance_ = createInstance(context_);
    debugMessenger_ = setupDebugMessenger(instance_);
    physicalDevice_ = pickPhysicalDevice(instance_);
}

void HelloTriangleApplication::mainLoop() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanup() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}
