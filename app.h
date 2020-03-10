#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.h"
#include "window.h"

#include <array>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <memory>

#include <glm/glm.hpp>

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

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
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
        void createSwapchain();
        void createImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createVertexBuffer();
        void createCommandPool();
        void createCommandBuffers();

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        Window *window;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue, computeQueue, presentQueue;

        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> swapchainFramebuffers;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        // Helpers
        std::vector<const char *> getRequiredExtensions();

        QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);

        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice);

        bool checkValidationLayerSupport();
};

#endif

