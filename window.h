#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H

#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Window {
    public:
        Window(void *app, int width, int height, std::string title, GLFWframebuffersizefun framebufferResizedCallback) : width(width), height(height), title(title) {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, app);
            glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
        }

        ~Window() {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        bool shouldClose() {
            return glfwWindowShouldClose(window);
        }

        void pollEvents() { glfwPollEvents(); }
        GLFWwindow *getWindow() { return window; }

        void getFramebufferSize(int *width, int *height) {
            glfwGetFramebufferSize(window, width, height);
        }

        std::vector<const char *> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            return extensions;
        }

    private:
        int width, height;
        std::string title;
        GLFWwindow *window;
};

#endif

