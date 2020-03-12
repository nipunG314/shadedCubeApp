#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform UniformBufferObject {
    vec3 lightDirX;
    vec3 lightDirY;
    vec3 lightDirZ;
    vec3 lightColor;
} lo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 coolColor = vec3(0.0, 0.0, 0.55) +  fragColor;
    vec3 unlitColor = 0.8 * coolColor;
    vec3 view = vec3(1.0f, 1.0f, 1.0f);
    float litColorX = dot(normalize(lo.lightDirX), normalize(inNormal));
    float litColorY = dot(normalize(lo.lightDirY), normalize(inNormal));
    float litColorZ = dot(normalize(lo.lightDirZ), normalize(inNormal));

    outColor = vec4(unlitColor * (1.0 + litColorX + litColorY + litColorZ) , 1.0);
}

