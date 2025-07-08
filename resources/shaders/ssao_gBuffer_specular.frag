#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in flat int fragMaterial;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outSpecular;

layout(binding = 0) uniform UniformGBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 nearFarPlanes;
} ubo;
layout(binding = 1) uniform sampler2D[] texSamplers;

// Propiedades del objeto
vec3 Ka; // Color ambiental del material
vec3 Kd; // Componente difusa del material
vec3 Ks; // Componente especular del material

void main() {

    uint materialId = fragMaterial * 3;

    float alpha = texture(texSamplers[nonuniformEXT(materialId + 1)], fragTexCoord).r;
    if (alpha < 0.1) discard;

    Kd = texture(texSamplers[nonuniformEXT(materialId)], fragTexCoord).rgb;
    Ks = texture(texSamplers[nonuniformEXT(materialId + 2)], fragTexCoord).rgb;

    outColor = vec4(Kd, 1.0);
    outSpecular = vec4(Ks, 1.0);
    
    // Se normaliza y se comprime la normal, de [-1,1] a [0,1]
    outNormal = vec4(normalize(fragNormal) * 0.5 + 0.5, 1.0);
    
    // Se almacena la posición, sumado a la profundidad lineal asociada, en el canal alfa
    outPosition = vec4(fragPosition, 1.0);
    
}

