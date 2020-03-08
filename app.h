#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.h"
#include "window.h"

#include <iostream>
#include <stdexcept>
#include <memory>

class VulkanSampleApp {
    public:
        VulkanSampleApp();
        ~VulkanSampleApp();
        void run();
    private:
        void createInstance();
        void setupDebugMessenger();
        void selectPhysicalDevice();

        Window *window;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        // Helpers
        std::vector<const char *> getRequiredExtensions();
        inline void handleVkResult(VkResult result, const char *message) {
            if (result != VK_SUCCESS)
                throw std::runtime_error(message);
        }
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

