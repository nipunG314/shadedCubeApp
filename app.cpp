#include "app.h"

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE = "Vulkan";

VulkanSampleApp::VulkanSampleApp() {
    window = new Window(WIDTH, HEIGHT, TITLE);
    createInstance();
}

VulkanSampleApp::~VulkanSampleApp() {
    vkDestroyInstance(instance, nullptr);
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
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    handleVkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Failed to create Vulkan Instance");
}

