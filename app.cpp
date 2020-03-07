#include "app.h"
#include "vulkan/vulkan.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE = "Vulkan";

VulkanSampleApp::VulkanSampleApp() {
    window.reset(new Window(WIDTH, HEIGHT, TITLE));
    createInstance();
}

void VulkanSampleApp::run() {
    while(!window->shouldClose()) {
        window->pollEvents();
    }
}

std::vector<const char *> VulkanSampleApp::getRequiredExtensions() {
    auto extensions = window->getRequiredExtensions();
    return extensions;
}

void VulkanSampleApp::createInstance() {
    vk::ApplicationInfo appInfo("Vulkan Sample App", 1, "", 1, VK_API_VERSION_1_2);
    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo instanceCreateInfo(
        {},
        &appInfo,
        0,
        nullptr,
        static_cast<uint32_t>(extensions.size()),
        extensions.data()
    );

    instance = vk::createInstanceUnique(instanceCreateInfo);
}

