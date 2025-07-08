#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in flat int fragMaterial;
layout(location = 4) in vec3 inViewVec;
layout(location = 5) in vec3 inLightVec;

layout(location = 0) out vec4 outColor;

layout (binding = 2) uniform accelerationStructureEXT topLevelAS;
layout(binding = 3) uniform sampler2D[] texSamplers;

float ambient = 0.1;

vec3 Ka;
vec3 Kd;
vec3 Ks;

float rugosityAlpha = 3.0;

vec3 rayDirection;

vec3 shadeMainLight();
bool isShadowed();

void main() {

    uint materialId = fragMaterial * 3;

    float alpha = texture(texSamplers[nonuniformEXT(materialId + 1)], fragTexCoord).r;
    if (alpha < 0.1) discard;

    vec3 finalColor = vec3(0.0);

    Ka = texture(texSamplers[nonuniformEXT(materialId)], fragTexCoord).rgb;
    Kd = texture(texSamplers[nonuniformEXT(materialId)], fragTexCoord).rgb;
    Ks = texture(texSamplers[nonuniformEXT(materialId + 2)], fragTexCoord).rgb;

    finalColor += shadeMainLight();
    if (isShadowed()) finalColor *= 0.1;

    outColor = vec4(finalColor, 1.0);

}

vec3 shadeMainLight() {

    vec3 c = vec3(0.0);

    // PARTE DIFUSA //
    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
    rayDirection = L;
    vec3 diffuse = Kd * max(dot(L,N), 0.0);
    c+= diffuse;

    // PARTE ESPECULAR // 
    vec3 V = normalize(inViewVec);
    vec3 R = normalize(-reflect(L,N));
    float factor = clamp(dot(R,V), 0.0, 1.0);
    vec3 specular = Ks * pow(factor, rugosityAlpha);
    c+= specular;

    // PARTE AMBIENTAL //
    c += ambient * Ka; 

    c = clamp(c, 0.0, 1.0);

    return c;

}

bool isShadowed() {
    // Se inicializa el rayo
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, inWorldPosition, 0.01, rayDirection, 5000.0);

    // Se procesa el rayo con la estructura de aceleración dada
    rayQueryProceedEXT(rayQuery);

    // Si el rayo interseca con algún triángulo antes de llegar a la luz, quiere decir que está sombreado
    if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) {
		return true;
    }
    return false;
}

