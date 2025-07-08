#version 450
#define NUM_LIGHTS 4

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 3) uniform UniformDataOffscreen {
    mat4 depthMVP;
} mainLightData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inMaterial;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out int fragMaterial;
layout(location = 4) out vec3 outViewVec;
layout(location = 5) out vec3 outLightVec;
layout(location = 6) out vec4 shadowCoord; // Coordenadas en espacio de luz, para shadow mapping

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

layout(binding = 2) uniform LightsData {
    Light lights[NUM_LIGHTS];
    MainDirectionalLight mainLight;
} sceneLights;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    // Se pasa la posición del vértice a coordenadas de la cámara
    fragPosition = (ubo.view * ubo.model * vec4(inPosition, 1.0)).xyz;
    // Se pasan las normales a coordenadas de la cámara
    fragNormal = (transpose(inverse(ubo.view * ubo.model)) * vec4(inNormal, 0.0)).xyz;
    // Se pasa el id del material
    fragMaterial = inMaterial;
    // Se calculan las coordenadas del vértice en espacio de la luz
    shadowCoord = biasMat * mainLightData.depthMVP * vec4(inPosition, 1.0);
    // Se calculan los vectores de la luz principal
    outLightVec = normalize((ubo.view * vec4(sceneLights.mainLight.position, 1.0)).xyz - inPosition);
    outViewVec = -inPosition;
}