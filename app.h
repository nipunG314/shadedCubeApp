#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#define GLFW_INCLUDE_VULKAN
#include "glfw-3.3.2/include/GLFW/glfw3.h"

#include <stdexcept>
#include <memory> 

class VulkanSampleApp {
    public:
        void run();
    private:
        void initWindow();
        void initVulkan();
        void mainLoop();
};

#endif

