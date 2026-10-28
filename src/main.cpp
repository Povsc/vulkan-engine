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

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<char const*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

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

    void initVulkan() { 
        createInstance();
        setupDebugMessenger();
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

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        // TODO: add runtime flag(?) to display more verbose output (think glog)
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
            .messageSeverity = severityFlags, .messageType = messageTypeFlags, .pfnUserCallback = &debugCallback};

        debugMessenger = instance_.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

    }


    void createInstance() {
        constexpr vk::ApplicationInfo appInfo{.pApplicationName = "Hello Triangle",
                                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                              .pEngineName = "No Engine",
                                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                              .apiVersion = vk::ApiVersion14};

        auto requiredExtensions = getRequiredInstanceExtensions();
        auto requiredValidationLayers = getRequiredValidationLayers();

        vk::InstanceCreateInfo createInfo{.pApplicationInfo = &appInfo,
                                          .enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size()),
                                          .ppEnabledLayerNames  = requiredValidationLayers.data(),
                                          .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
                                          .ppEnabledExtensionNames = requiredExtensions.data()};

        instance_ = vk::raii::Instance(context_, createInfo);
    }

    std::vector<const char*> getRequiredInstanceExtensions() {
        // get required extensions from GLFW (needed since vk is platform agnostic)
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // conditionally add extension for annotated callbacks from validation layers
        if (enableValidationLayers) {
            requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        // check if extensions are supported by Vulkan implementation
        auto extensionProperties = context_.enumerateInstanceExtensionProperties();
        auto unsupportedPropertyIt =
            std::ranges::find_if(requiredExtensions, [&extensionProperties](auto const& requiredExtension) {
                return std::ranges::none_of(extensionProperties, [requiredExtension](auto const& extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });

        if (unsupportedPropertyIt != requiredExtensions.end()) {
            throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
        }

        return requiredExtensions;
    }

    std::vector<const char*> getRequiredValidationLayers() {
        // get required layers
        std::vector<char const*> requiredLayers;
        if (enableValidationLayers) {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

        // check if the required layers are supported by Vulkan implementation.
        auto layerProperties = context_.enumerateInstanceLayerProperties();
        auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](auto const& requiredLayer) {
            return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
        });

        if (unsupportedLayerIt != requiredLayers.end()) {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }

        return requiredLayers;
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void* pUserData) {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }

    GLFWwindow* window_;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
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