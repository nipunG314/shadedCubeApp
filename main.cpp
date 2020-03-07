#include <exception>
#include <iostream>

#include "app.h"

int main() {
    VulkanSampleApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

