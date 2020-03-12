#ifndef VULKAN_CONSTANTS_H
#define VULKAN_CONSTANTS_H

#include "vulkan/include/vulkan/vulkan.h"
#include "app.h"

#include <string>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE = "Vulkan";

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Vertex Input data
const std::vector<Vertex> vertices = {
    {{0.5f, 0.5f, 0.5f},    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
    {{0.5f, -0.5f, 0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f},  {0.0f, 1.0f, 1.0f}, {1.0f, -1.0f, -1.0f}},
    {{-0.5f, 0.5f, 0.5f},   {1.0f, 0.0f, 0.0f}, {-1.0f, 1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f},  {1.0f, 0.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}},
    {{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}}
};

// Index Buffer data
const std::vector<uint16_t> indices = {
    4, 2, 0,
    2, 4, 6,
    1, 3, 5,
    7, 5, 3,
    6, 4, 5,
    5, 7, 6,
    3, 1, 0,
    0, 2, 3,
    5, 4, 0,
    0, 1, 5,
    3, 2, 6,
    6, 7, 3
};

const int MAX_FRAMES_IN_FLIGHT = 10;

#endif

