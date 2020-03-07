#ifndef VULKAN_APP_H
#define VULKAN_APP_H

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

