#include <exception>
#include <iostream>

#include "app.h"

int main() {
    try {
        VulkanSampleApp app;
        app.run();
    } catch (vk::SystemError& err) {
        std::cerr << "vk::SystemError: " << err.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

