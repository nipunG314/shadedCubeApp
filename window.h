#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include "glfw-3.3.2/include/GLFW/glfw3.h"

#include <string>

class Window {
    public:
        Window(int width, int height, std::string title) : width(width), height(height), title(title) {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        }

        ~Window() {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        bool shouldClose() {
            return glfwWindowShouldClose(window);
        }

        void pollEvents() { glfwPollEvents(); }

    private:
        int width, height;
        std::string title;
        GLFWwindow *window;
};

#endif

