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
#include <map>

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;

#ifdef NDEBUG
constexpr bool DEBUG = false;
#else
constexpr bool DEBUG = true;
#endif

const std::vector<char const*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

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
        pickPhysicalDevice();
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
        if (!DEBUG) return;

        // TODO: add runtime flag(?) to display more verbose output (think glog)
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
            .messageSeverity = severityFlags, .messageType = messageTypeFlags, .pfnUserCallback = &debugCallback};

        debugMessenger_ = instance_.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

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

    void pickPhysicalDevice() { 
        auto physicalDevices = instance_.enumeratePhysicalDevices();

        if (DEBUG) {
            std::cout << "Physical devices available: \n";
            for (const auto& physicalDevice : physicalDevices) {
                std::cout << " * " << physicalDevice.getProperties().deviceName << "\n";
            }
        }

        std::multimap<int, vk::raii::PhysicalDevice> candidates;

        // if many discrete GPUs this could get annoying, but good enough for now
        for (const auto& physicalDevice : physicalDevices) {
            if (!isDeviceSuitable(physicalDevice)) {
                continue;
            }

            auto deviceProperties = physicalDevice.getProperties();
            // auto deviceFeatures = physicalDevice.getFeatures();
            uint32_t score = 0;

            // heavily favor discrete GPU
            if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                score += 1000;
            }

            score += deviceProperties.limits.maxImageDimension2D;

            candidates.insert(std::make_pair(score, physicalDevice));
        }

        if (!candidates.empty() && candidates.rbegin()->first > 0) {
            physicalDevice_ = candidates.rbegin()->second;
        } else {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
    }

    bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
        bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

        // could be advantageous in the future to check if other queue families are supported
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(
            queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

        // check if required physicalDevice extensions are available
        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            requiredDeviceExtension, [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
                return std::ranges::any_of(
                    availableDeviceExtensions, [requiredDeviceExtension](auto const& availableDeviceExtension) {
                        return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                    });
            });

        // check if the physicalDevice supports the required features (shader draw parameters, dynamic rendering and
        // extended dynamic state)
        auto features =
            physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                 vk::PhysicalDeviceVulkan13Features,
                                                 vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    }

    std::vector<const char*> getRequiredInstanceExtensions() {
        // get required extensions from GLFW (needed since vk is platform agnostic)
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // conditionally add extension for annotated callbacks from validation layers
        if (DEBUG) {
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
        if (DEBUG) {
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

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback([[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          [[maybe_unused]] void* pUserData) {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }

    GLFWwindow* window_;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::raii::PhysicalDevice physicalDevice_ = nullptr;
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