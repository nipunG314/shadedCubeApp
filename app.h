#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#include "vulkan/include/vulkan/vulkan.h"
#include "window.h"

#include <array>
#include <glm/detail/type_mat.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <memory>

#include <glm/glm.hpp>
#include <vector>

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
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }
};

struct UniformTransformObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct UniformLightObject {
    glm::vec3 lightDir;
    glm::vec3 lightColor;
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
        void createDescriptorSetLayouts();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformTransforms();
        void createUniformLights();
        void updateUniforms(uint32_t currentFrame);
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandPool();
        void createCommandBuffers();
        void cleanupSwapchain();
        void recreateSwapchain();
        void createSyncObjects();

        void drawFrame();

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
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        std::vector<VkBuffer> uniformTransforms;
        std::vector<VkDeviceMemory> uniformTransformsMemory;
        std::vector<VkBuffer> uniformLights;
        std::vector<VkDeviceMemory> uniformLightsMemory;

        VkDescriptorPool descriptorPool;
        std::vector<std::vector<VkDescriptorSet>> descriptorSets;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderCompleteSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
        bool framebufferResized = false;

        // Helpers
        std::vector<const char *> getRequiredExtensions();

        QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);

        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice);

        bool checkValidationLayerSupport();
};

#endif

