#include "app.h"
#include "helpers.h"
#include "constants.h"

#include <cstdlib>
#include <cstring>
#include <glm/detail/type_vec.hpp>
#include <set>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

VulkanSampleApp::VulkanSampleApp(ShaderProgram program) : shaderProgram(program) {
    auto framebufferResizedCallback = [](GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<VulkanSampleApp *>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    };
    window = new Window(
        this,
        WIDTH,
        HEIGHT,
        TITLE,
        static_cast<GLFWframebuffersizefun>(framebufferResizedCallback)
    );
    createInstance();
    setupDebugMessenger();
    createSurface();
    selectPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayouts();
    createGraphicsPipeline();
    createFramebuffers();
    createVertexBuffer();
    createIndexBuffer();
    createUniformTransforms();
    createUniformLights();
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

VulkanSampleApp::~VulkanSampleApp() {
    cleanupSwapchain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[0], nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[1], nullptr);
    for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderCompleteSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanSampleApp::run() {
    while(!window->shouldClose()) {
        window->pollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

std::vector<const char *> VulkanSampleApp::getRequiredExtensions() {
    auto extensions = window->getRequiredExtensions();

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    
        std::cout << "Required Extensions:\n";
        for(const char *extensionName: extensions)
            std::cout << extensionName << "\n";
    }
    return extensions;
}

bool VulkanSampleApp::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char *layerName: validationLayers) {
        bool layerFound = false;

        for(auto& layer: availableLayers) {
            if (strcmp(layerName, layer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            std::cout << "Available Layers:\n";
            for(auto& layer: availableLayers) {
                std::cout << layer.layerName << "\n";
            }
            std::cout << "Layers required:\n";
            for(const char *layerName: validationLayers) {
                std::cout << layerName << "\n";
            }
            return false;
        }

    }

    return true;
}

void VulkanSampleApp::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("Validation layers requested but not availble!");

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
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    handleVkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Failed to create Vulkan Instance");
}

void VulkanSampleApp::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    handleVkResult(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "Failed to setup Debug Messenger!");
}

QueueFamilyIndices VulkanSampleApp::findQueueFamilyIndices(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

    int i=0;
    for(const auto& queue: properties) {
        if (!indices.graphicsQueue.has_value() && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsQueue = i;
        if (!indices.computeQueue.has_value() && queue.queueFlags & VK_QUEUE_COMPUTE_BIT)
            indices.computeQueue = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentQueue = i;

        if (indices.has_value())
            break;

        i++;
    }

    return indices;
}

void VulkanSampleApp::createSurface() {
   handleVkResult(glfwCreateWindowSurface(instance, window->getWindow(), nullptr, &surface), "Failed to create Window Surface!");
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension: availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

bool VulkanSampleApp::isDeviceSuitable(VkPhysicalDevice device) {
    // Add suitability checks as the need arises
    auto indices = findQueueFamilyIndices(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapchainAdequate = false;
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);

    if (extensionsSupported) {
        swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }

    if (enableValidationLayers) {
        std::cout << "QueueFamilyIndices:\n";
        std::cout << "GraphicsQueue: " << indices.graphicsQueue.has_value() << "\n";
        std::cout << "ComputeQueue: " << indices.computeQueue.has_value() << "\n";
        std::cout << "QueueFamilyIndices: " << indices.has_value() << "\n";
        std::cout << "extensionsSupported: " << extensionsSupported << "\n";
        std::cout << "swapchainAdequate: " << swapchainAdequate << "\n";
        std::cout << "swapchainSupport.formats.size(): " << swapchainSupport.formats.size() << "\n";
        std::cout << "swapchainSupport.presentModes.size(): " << swapchainSupport.presentModes.size() << "\n";
    }

    return indices.has_value() && extensionsSupported && swapchainAdequate;
}

void VulkanSampleApp::selectPhysicalDevice() {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("Failed to find any Vulkan-compatible physical devices!");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    std::cout << "Found " << deviceCount << " physical devices!\n";
    for(const auto& device: physicalDevices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable physical device!");
}

void VulkanSampleApp::createLogicalDevice() {
    auto indices = findQueueFamilyIndices(physicalDevice);
    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsQueue.value(),
        indices.computeQueue.value(),
        indices.presentQueue.value()
    };

    for(uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures features = {};

    // Device Create Info
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &features;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Add Device-level validation layers
    // to comply with out-of-date Vulkan
    // implementations
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else
        createInfo.enabledLayerCount = 0;

    handleVkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr,&device), "Failed to create Logical Device!");
    vkGetDeviceQueue(device, indices.graphicsQueue.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.computeQueue.value(), 0, &computeQueue);
    vkGetDeviceQueue(device, indices.presentQueue.value(), 0, &presentQueue);
}

SwapchainSupportDetails VulkanSampleApp::querySwapchainSupport(VkPhysicalDevice device) {
    SwapchainSupportDetails details;

    // Surface Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // Surface Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // Surface Presentation Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


void VulkanSampleApp::createSwapchain() {
    auto swapchainSupport = querySwapchainSupport(physicalDevice);
    auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    auto extent = chooseSwapExtent(window, swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        imageCount = swapchainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue Sharing for the Swapchain
    auto indices = findQueueFamilyIndices(physicalDevice);
    uint32_t queueFamilyIndices[] = {
        indices.graphicsQueue.value(),
        indices.presentQueue.value()
    };

    if (indices.graphicsQueue != indices.presentQueue) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    handleVkResult(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain), "Failed to create Swap Chain!");

   vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
   swapchainImages.resize(imageCount);
   vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
}

void VulkanSampleApp::createImageViews() {
    swapchainImageViews.resize(swapchainImages.size());

    for(size_t i = 0; i< swapchainImages.size(); i++) {
       VkImageViewCreateInfo createInfo = {};
       createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
       createInfo.image = swapchainImages[i];
       createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
       createInfo.format = swapchainImageFormat;
       createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
       createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
       createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
       createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
       createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       createInfo.subresourceRange.baseMipLevel = 0;
       createInfo.subresourceRange.levelCount = 1;
       createInfo.subresourceRange.baseArrayLayer = 0;
       createInfo.subresourceRange.layerCount = 1;

       handleVkResult(vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]), "Failed to create an ImageView!");
    }
}

void VulkanSampleApp::createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    handleVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass), "Failed to create Render Pass!");
}

void VulkanSampleApp::createDescriptorSetLayouts() {
    descriptorSetLayouts.resize(2);

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags =  VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    handleVkResult(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[0]), "Failed to create a Descriptor Set Layout!");

    VkDescriptorSetLayoutBinding loLayoutBinding = {};
    loLayoutBinding.binding = 0;
    loLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    loLayoutBinding.descriptorCount = 1;
    loLayoutBinding.stageFlags =  VK_SHADER_STAGE_FRAGMENT_BIT;

    layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &loLayoutBinding;

    handleVkResult(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[1]), "Failed to create a Descriptor Set Layout!");
}

void VulkanSampleApp::createGraphicsPipeline() {
    auto vertShaderModule = createShaderModule(device, "shaders/vert.spv");
    auto fragShaderModule = createShaderModule(device, "shaders/frag.spv");
    if (shaderProgram == ShaderProgram::BRIGHT_SHADER) {
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        fragShaderModule = createShaderModule(device, "shaders/bright.spv");
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStageInfos[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    if (enableValidationLayers) {
        std::cout << "AttributeDescriptions:\n";
        for(size_t i=0; i<attributeDescriptions.size(); i++) {
            std::cout << "AttributeDescription #" << i+1 << "\n";

            std::cout << "description.format: " << attributeDescriptions[i].format << "\n";

            std::cout << "\n";
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchainExtent.width;
    viewport.height = (float) swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    handleVkResult(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageInfos;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional

    pipelineInfo.layout = pipelineLayout;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    handleVkResult(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline), "Failed to create Graphics Pipeline!");

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanSampleApp::createFramebuffers() {
    swapchainFramebuffers.resize(swapchainImageViews.size());
    for(size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        handleVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]), "Failed to create Framebuffer!");
    }
}

void VulkanSampleApp::createVertexBuffer() {
    // Creating and Allocating the Vertex Buffer
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory
    );
    // Filling the Vertex Buffer
    void *data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, vertexBufferMemory);
}

void VulkanSampleApp::createIndexBuffer() {
    // Creating and Allocating the Index Buffer
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    createBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer,
        indexBufferMemory
    );
    // Filling the Index Buffer
    void *data;
    vkMapMemory(device, indexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, indexBufferMemory);
}

void VulkanSampleApp::createUniformTransforms() {
    VkDeviceSize bufferSize = sizeof(UniformTransformObject);

    uniformTransforms.resize(swapchainImages.size());
    uniformTransformsMemory.resize(swapchainImages.size());

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memRequirements;

    for(size_t i=0; i< swapchainImages.size(); i++) {
        createBuffer(
            physicalDevice,
            device,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformTransforms[i],
            uniformTransformsMemory[i]
        );
    }
}

void VulkanSampleApp::createUniformLights() {
    VkDeviceSize bufferSize = sizeof(UniformLightObject);

    uniformLights.resize(swapchainImages.size());
    uniformLightsMemory.resize(swapchainImages.size());

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memRequirements;

    for(size_t i=0; i< swapchainImages.size(); i++) {
        createBuffer(
            physicalDevice,
            device,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformLights[i],
            uniformLightsMemory[i]
        );
    }
}

void VulkanSampleApp::updateUniforms(uint32_t currentFrame) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformTransformObject ubo = {};
    ubo.model = glm::rotate(
        glm::mat4(1.0f),
        time * glm::radians(90.f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    ubo.proj = glm::perspective(
        glm::radians(45.0f),
        swapchainExtent.width / (float) swapchainExtent.height,
        0.1f,
        10.0f
    );
    ubo.proj[1][1] *= -1;

    UniformLightObject lo = {};
    lo.lightDirX = glm::vec3(-1.0f, -1.0f, -1.0f);
    lo.lightDirY = glm::vec3(1.0f, 1.0f, 1.0f);
    lo.lightDirZ = glm::vec3(0.5f, 0.0f, 0.5f);
    lo.lightColor = glm::vec3(0.01f);

    void *data;
    vkMapMemory(device, uniformTransformsMemory[currentFrame], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformTransformsMemory[currentFrame]);
    vkMapMemory(device, uniformLightsMemory[currentFrame], 0, sizeof(lo), 0, &data);
    memcpy(data, &lo, sizeof(lo));
    vkUnmapMemory(device, uniformLightsMemory[currentFrame]);
}

void VulkanSampleApp::createDescriptorPool() {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapchainImages.size() * 2);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapchainImages.size() * 2);

    handleVkResult(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Failed to create descriptor pool!");
}

void VulkanSampleApp::createDescriptorSets() {
    descriptorSetLayouts.resize(swapchainImages.size());

    std::vector<VkDescriptorSetLayout> vertexLayouts(swapchainImages.size(), descriptorSetLayouts[0]);
    std::vector<VkDescriptorSetLayout> fragmentLayouts(swapchainImages.size(), descriptorSetLayouts[1]);
    std::vector<VkDescriptorSet> transformDescriptorSets(swapchainImages.size());
    std::vector<VkDescriptorSet> lightDescriptorSets(swapchainImages.size());

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImages.size());
    allocInfo.pSetLayouts = vertexLayouts.data();

    handleVkResult(vkAllocateDescriptorSets(device, &allocInfo, transformDescriptorSets.data()), "Failed to allocate descriptor sets!");

    for(size_t i=0; i<swapchainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformTransforms[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformTransformObject);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = transformDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    allocInfo.pSetLayouts = fragmentLayouts.data();

    handleVkResult(vkAllocateDescriptorSets(device, &allocInfo, lightDescriptorSets.data()), "Failed to allocate descriptor sets!");

    for(size_t i=0; i<swapchainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformLights[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformLightObject);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = lightDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    for(size_t i=0; i<swapchainImages.size(); i++) {
        descriptorSets.push_back({transformDescriptorSets[i], lightDescriptorSets[i]});
    }
}

void VulkanSampleApp::createCommandPool() {
    auto queueFamilyIndices = findQueueFamilyIndices(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsQueue.value();
    poolInfo.flags = 0;

    handleVkResult(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "Failed to create command pool!");
}

void VulkanSampleApp::createCommandBuffers() {
    commandBuffers.resize(swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    handleVkResult(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()), "Failed to allocate command buffers!");

    for(size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        handleVkResult(vkBeginCommandBuffer(commandBuffers[i], &beginInfo),"Failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2, descriptorSets[i].data(), 0, nullptr);
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        handleVkResult(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer!");
    }
}


void VulkanSampleApp::cleanupSwapchain() {
    for(auto framebuffer : swapchainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    for(size_t i=0; i<swapchainImages.size(); i++) {
        vkDestroyBuffer(device, uniformLights[i], nullptr);
        vkFreeMemory(device, uniformLightsMemory[i], nullptr);
        vkDestroyBuffer(device, uniformTransforms[i], nullptr);
        vkFreeMemory(device, uniformTransformsMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for(auto imageView : swapchainImageViews)
        vkDestroyImageView(device, imageView, nullptr);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void VulkanSampleApp::recreateSwapchain() {
    vkDeviceWaitIdle(device);

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createUniformTransforms();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void VulkanSampleApp::createSyncObjects() {
   imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   renderCompleteSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
   imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

   VkSemaphoreCreateInfo semaphoreCreateInfo = {};
   semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   VkFenceCreateInfo fenceCreateInfo = {};
   fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
       handleVkResult(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), "Failed to create imageAvailableSemaphore!");
       handleVkResult(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphores[i]), "Failed to create renderCompleteSemaphores!");
       handleVkResult(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]), "Failed to create fence!");
   }
}

void VulkanSampleApp::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire Swapchain image!");

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniforms(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderCompleteSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    handleVkResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    } else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to present Swapchain image!");

    currentFrame = (currentFrame+1) % MAX_FRAMES_IN_FLIGHT;
}

