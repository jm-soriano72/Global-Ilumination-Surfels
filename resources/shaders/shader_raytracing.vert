#version 450
#define NUM_LIGHTS 4

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterial;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 outWorldPosition;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out int fragMaterial;
layout(location = 4) out vec3 outViewVec;
layout(location = 5) out vec3 outLightVec;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

struct Light {
    vec3 position;
    float intensity;
    vec3 color;
};

struct MainDirectionalLight {
    vec3 position;
    vec3 target;
    vec3 direction;
    float intensity;
};

layout(binding = 1) uniform LightsData {
    Light lights[NUM_LIGHTS];
    MainDirectionalLight mainLight;
} sceneLights;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    fragTexCoord = inTexCoord;
    fragMaterial = inMaterial;

    // Se deja la posición en coordenadas globales para el trazado de rayos
    outWorldPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;

    // Las normales también se dejan en coordenadas del mundo
    outNormal = (ubo.model * vec4(inNormal, 0.0)).xyz;

    // Se calculan los vectores de la luz principal
    outLightVec = normalize(sceneLights.mainLight.position - inPosition);
    outViewVec = -inPosition;
}