#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.h"
#include "window.h"

#include <stdexcept>
#include <memory>

class VulkanSampleApp {
    public:
        VulkanSampleApp();
        ~VulkanSampleApp();
        void run();
    private:
        // Helpers
        std::vector<const char *> getRequiredExtensions();
        inline void handleVkResult(VkResult result, const char *message) {
            if (result != VK_SUCCESS)
                throw std::runtime_error(message);
        }
        bool checkValidationLayerSupport();

        void createInstance();

        Window *window;
        VkInstance instance;
};

#endif

