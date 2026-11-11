#include "vulkan/device.hpp"

#include "vulkan/config.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {

bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    // could be advantageous in the future to check if other queue families are supported
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(
        queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension, [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
            return std::ranges::any_of(availableDeviceExtensions,
                                       [requiredDeviceExtension](auto const& availableDeviceExtension) {
                                           return strcmp(availableDeviceExtension.extensionName,
                                                         requiredDeviceExtension) == 0;
                                       });
        });

    // shader draw parameters, dynamic rendering, extended dynamic state
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

} // namespace

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance& instance) {
    auto physicalDevices = instance.enumeratePhysicalDevices();

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
        uint32_t score = 0;

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        score += deviceProperties.limits.maxImageDimension2D;

        candidates.insert(std::make_pair(score, physicalDevice));
    }

    if (!candidates.empty() && candidates.rbegin()->first > 0) {
        return candidates.rbegin()->second;
    }

    throw std::runtime_error("failed to find GPUs with Vulkan support!");
}

vk::raii::Device createLogicalDevice(vk::raii::PhysicalDevice& physicalDevice, uint32_t& graphicsFamilyIndex) {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const& qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
    });
    if (graphicsQueueFamilyProperty == queueFamilyProperties.end()) {
        throw std::runtime_error("failed to find a graphics queue family");
    }
    graphicsFamilyIndex =
        static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

    // a little silly since only one queue
    const float queuePriority = 0.5f;

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        .queueFamilyIndex = graphicsFamilyIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};


    // are any of these actually needed?
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2, 
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features, 
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {},                             // vk::PhysicalDeviceFeatures2 (empty for now)
            {.shaderDrawParameters = true}, // enable shader draw parameters from Vulkan 1.1
            {.dynamicRendering = true},     // enable dynamic rendering from Vulkan 1.3
            {.extendedDynamicState = true}  // enable extended dynamic state from the extension
        };

    vk::DeviceCreateInfo deviceCreateInfo{.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                          .queueCreateInfoCount = 1,
                                          .pQueueCreateInfos = &deviceQueueCreateInfo,
                                          .enabledExtensionCount =
                                              static_cast<uint32_t>(requiredDeviceExtension.size()),
                                          .ppEnabledExtensionNames = requiredDeviceExtension.data()};

    return vk::raii::Device(physicalDevice, deviceCreateInfo);
}