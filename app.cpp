#include "app.h"
#include <cstring>
#include <set>

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE = "Vulkan";

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT"
    );
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT"
    );
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

VulkanSampleApp::VulkanSampleApp() {
    window = new Window(WIDTH, HEIGHT, TITLE);
    createInstance();
    setupDebugMessenger();
    createSurface();
    selectPhysicalDevice();
    createLogicalDevice();
}

VulkanSampleApp::~VulkanSampleApp() {
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanSampleApp::run() {
    while(!window->shouldClose()) {
        window->pollEvents();
    }
}

std::vector<const char *> VulkanSampleApp::getRequiredExtensions() {
    auto extensions = window->getRequiredExtensions();

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    
        std::cout << "Required Extensions:\n";
        for(const char *extensionName: extensions)
            std::cout << extensionName << "\n";
    }
    return extensions;
}

bool VulkanSampleApp::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char *layerName: validationLayers) {
        bool layerFound = false;

        for(auto& layer: availableLayers) {
            if (strcmp(layerName, layer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            std::cout << "Available Layers:\n";
            for(auto& layer: availableLayers) {
                std::cout << layer.layerName << "\n";
            }
            std::cout << "Layers required:\n";
            for(const char *layerName: validationLayers) {
                std::cout << layerName << "\n";
            }
            return false;
        }

    }

    return true;
}

void VulkanSampleApp::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("Validation layers requested but not availble!");

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanSampleApp";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.flags = {};
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    handleVkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Failed to create Vulkan Instance");
}

void VulkanSampleApp::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    handleVkResult(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "Failed to setup Debug Messenger!");
}

QueueFamilyIndices VulkanSampleApp::findQueueFamilyIndices(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

    int i=0;
    for(const auto& queue: properties) {
        if (!indices.graphicsQueue.has_value() && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsQueue = i;
        if (!indices.computeQueue.has_value() && queue.queueFlags & VK_QUEUE_COMPUTE_BIT)
            indices.computeQueue = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentQueue = i;

        if (indices.has_value())
            break;

        i++;
    }

    return indices;
}

void VulkanSampleApp::createSurface() {
   handleVkResult(glfwCreateWindowSurface(instance, window->getWindow(), nullptr, &surface), "Failed to create Window Surface!");
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension: availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

bool VulkanSampleApp::isDeviceSuitable(VkPhysicalDevice device) {
    // Add suitability checks as the need arises
    auto indices = findQueueFamilyIndices(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    if (enableValidationLayers) {
        std::cout << "QueueFamilyIndices:\n";
        std::cout << "GraphicsQueue: " << indices.graphicsQueue.has_value() << "\n";
        std::cout << "ComputeQueue: " << indices.computeQueue.has_value() << "\n";
        std::cout << "QueueFamilyIndices: " << indices.has_value() << "\n";
    }

    return indices.has_value() && extensionsSupported;
}

void VulkanSampleApp::selectPhysicalDevice() {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("Failed to find any Vulkan-compatible physical devices!");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for(const auto& device: physicalDevices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable physical device!");
}

void VulkanSampleApp::createLogicalDevice() {
    auto indices = findQueueFamilyIndices(physicalDevice);
    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsQueue.value(),
        indices.computeQueue.value(),
        indices.presentQueue.value()
    };

    for(uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features = {};

    // Device Create Info
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &features;
    createInfo.enabledExtensionCount = 0;
    // Add Swapchain extensions on adding the Swapchain
    createInfo.enabledExtensionCount = 0;

    // Add Device-level validation layers
    // to comply with out-of-date Vulkan
    // implementations
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else
        createInfo.enabledLayerCount = 0;

    handleVkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr,&device), "Failed to create Logical Device!");
    vkGetDeviceQueue(device, indices.graphicsQueue.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.computeQueue.value(), 0, &computeQueue);
    vkGetDeviceQueue(device, indices.presentQueue.value(), 0, &presentQueue);
}

