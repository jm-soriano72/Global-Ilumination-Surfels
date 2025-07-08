#version 450

layout(binding = 0) uniform UniformDataOffscreen {
    mat4 depthMVP;
} mainLightData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterial;

void main() {
    gl_Position = mainLightData.depthMVP * vec4(inPosition, 1.0);
}