#include <exception>
#include <iostream>

#include "constants.h"
#include "app.h"

int main(int argc, char **argv) {
    ShaderProgram shaderProgram = DIFFUSE_SHADER;

    if (argc >= 2) {
        auto shaderProgramName = argv[1];
        for (ShaderProgram program = ShaderProgram::DIFFUSE_SHADER; program != ShaderProgram::END_OF_SHADERS; program = static_cast<ShaderProgram>(program + 1)) {
            if (validateShaderName(program, shaderProgramName)) {
                shaderProgram = program;
                break;
            }
        }
    }

    try {
        ShadedCubeApp app(shaderProgram);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

