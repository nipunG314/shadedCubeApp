#include "app.h"

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE = "Vulkan";

VulkanSampleApp::VulkanSampleApp() {
    window.reset(new Window(WIDTH, HEIGHT, TITLE));
}

void VulkanSampleApp::run() {
    while(!window->shouldClose()) {
        window->pollEvents();
    }
}

