#version 450

layout(binding = 0) uniform UniformGBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 nearFarPlanes;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterial;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out int fragMaterial;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    // Se pasa la posición del vértice a coordenadas de la cámara
    fragPosition = vec3(ubo.view * ubo.model * vec4(inPosition, 1.0));
    // Se pasan las normales a coordenadas de la cámara
    mat3 normalMatrix = transpose(inverse(mat3(ubo.view * ubo.model)));
    fragNormal = normalMatrix * inNormal;
    // Se pasa el id del material
    fragMaterial = inMaterial;
}