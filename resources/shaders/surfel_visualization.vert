#version 450

// Atributos
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in float inRadius;
layout(location = 3) in vec3 inColor;

// Variables que se pasan al Geometry Shader
layout(location = 0) out VS_OUT {
    vec3 pos;
    vec3 normal;
    float radius;
    vec3 color;
} vs_out;

// UBO con la información de la cámara
layout (binding = 0) uniform CameraBuffer {
	mat4 view;
    	mat4 projection;
    	vec2 nearFarPlanes;
        vec2 padding0;
    	vec4 frame;
        vec3 position;
        float padding1;
} cameraData;

void main() {
	
    vs_out.pos = inPos;
    vs_out.normal = normalize(inNormal);
    vs_out.radius = inRadius;
    vs_out.color = inColor;

    // Se proyecta la posición del surfel
    gl_Position   = cameraData.projection * cameraData.view * vec4(inPos, 1.0);
}