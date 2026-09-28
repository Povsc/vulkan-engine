#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string>

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;

class HelloTriangleApplication {
  public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    // TODO: reuse Window;Camera;Helper classes from my OpenGL project
    void initWindow() { 
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulcão", nullptr, nullptr);
    }

    void initVulkan() { createInstance();
    }

    void mainLoop() { 
        while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();
        } 
    }

    void cleanup() {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void createInstance() {
        constexpr vk::ApplicationInfo appInfo{.pApplicationName = "Hello Triangle",
                                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                              .pEngineName = "No Engine",
                                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                              .apiVersion = vk::ApiVersion14};

        // get required extensions from GLFW (needed since vk is platform agnostic)
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // check if extensions are supported by Vulkan implementation
        auto extensionProperties = context_.enumerateInstanceExtensionProperties();
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            if (std::ranges::none_of(extensionProperties, 
                [glfwExtension = glfwExtensions[i]](auto const& extensionProperty) {
                                              return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
                                          })) {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }

        vk::InstanceCreateInfo createInfo{.pApplicationInfo = &appInfo,
                                          .enabledExtensionCount = glfwExtensionCount,
                                          .ppEnabledExtensionNames = glfwExtensions};

        instance_ = vk::raii::Instance(context_, createInfo);
    }

    GLFWwindow* window_;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
};

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}