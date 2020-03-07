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
        std::shared_ptr<Window> window;
};

#endif

