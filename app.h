#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.h"
#include "window.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <memory>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsQueue;
    std::optional<uint32_t> computeQueue;
    std::optional<uint32_t> presentQueue;

    bool has_value() {
        return graphicsQueue.has_value() && computeQueue.has_value() && presentQueue.has_value();
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSampleApp {
    public:
        VulkanSampleApp();
        ~VulkanSampleApp();
        void run();
    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        Window *window;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue, computeQueue, presentQueue;

        // Helpers
        std::vector<const char *> getRequiredExtensions();

        inline void handleVkResult(VkResult result, const char *message) {
            if (result != VK_SUCCESS)
                throw std::runtime_error(message);
        }

        QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);

        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice);

        bool checkValidationLayerSupport();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {
                std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

                return VK_FALSE;
        }
};

#endif

