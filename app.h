#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.hpp"
#include "window.h"

#include <stdexcept>
#include <memory>

class VulkanSampleApp {
    public:
        VulkanSampleApp();
        void run();
    private:
        // Helpers
        std::vector<const char *> getRequiredExtensions();

        void createInstance();

        std::unique_ptr<Window> window;
        vk::UniqueInstance instance;
};

#endif

