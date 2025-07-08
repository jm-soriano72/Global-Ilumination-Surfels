#version 450
#define NUM_LIGHTS 4
layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (binding = 3) uniform sampler2D samplerSSAOBlur;
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
layout(binding = 4) uniform LightsData {
    Light lights[NUM_LIGHTS];
    MainDirectionalLight mainLight;
} sceneLights;
layout(binding = 5) uniform UniformDataOffscreen {
    mat4 depthMVP;
} mainLightData;
layout(binding = 6) uniform sampler2D shadowMap;
layout(binding = 7) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout (binding = 8) uniform sampler2D samplerSpecular;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

vec3 fragPos;
vec3 fragNormal;
const float rugosityAlpha = 3.0;
vec4 shadowCoord;

vec4 albedo;
vec4 specular;

const vec3 ambientIntensity = vec3(0.01);
float ssao;

float shadowCalculation(vec4 shadowCoordinates, vec2 off) {
 
    float closestDepth = texture( shadowMap, shadowCoordinates.xy + off ).r;
    float currentDepth = shadowCoordinates.z;
    float bias = 0.005;
    float shadow = currentDepth < closestDepth + bias ? 1.0 : 0.0;
    return shadow;
}

float shadowPCFCalculation(vec4 shadowCoordinates) {

    float scale = 1.5;
    ivec2 texDim = textureSize(shadowMap, 0);

    float dx = scale / float(texDim.x);
    float dy = scale / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    float range = 1.5;

    for(float x = -range; x <= range; x += 0.25) {
        for(float y = -range; y <= range; y += 0.25) {
            shadowFactor += shadowCalculation(shadowCoordinates, vec2(dx*x, dy*y));
            count++;
        }
     }

     return shadowFactor/count;
}

vec3 shadeMainLight() {

    vec3 c = vec3(0.0); // Color negro por defecto
    
    // Se calculan los vectores de la luz principal
    vec3 lightVec = normalize((ubo.view * vec4(sceneLights.mainLight.position, 1.0)).xyz - fragPos);
    vec3 viewVec = -fragPos;

    // PARTE DIFUSA //
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightVec);
    vec3 diffuse = sceneLights.mainLight.intensity * albedo.rgb * max(dot(L,N), 0.0);
    c+= diffuse;

    // PARTE ESPECULAR // 
    vec3 V = normalize(viewVec);
    vec3 R = normalize(-reflect(L,N));
    float factor = clamp(dot(R,V), 0.0, 1.0);
    vec3 specularComponent = sceneLights.mainLight.intensity * specular.rgb * pow(factor, rugosityAlpha);
    c+= specularComponent;

    float shadowFactor = shadowPCFCalculation(shadowCoord/shadowCoord.w);
    c *= shadowFactor; // Cálculo del factor de sombreado

    // PARTE AMBIENTAL //
    // Se añade la oclusión ambiental calculada
    c += ambientIntensity * albedo.rgb * ssao;

    c = clamp(c, 0.0, 1.0);

    return c;

}


void main() 
{
	fragPos = texture(samplerPosition, inUV).rgb;
	fragNormal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);
	albedo = texture(samplerAlbedo, inUV);
	specular = texture(samplerSpecular, inUV); 
	ssao = texture(samplerSSAOBlur, inUV).r;

	// Cálculo del shadow mapping //
        // Se revierte la transformación a espacio de cámara: la posición del fragmento se pasa a coordenadas del mundo              
	vec4 fragWorldPos = inverse(ubo.view) * vec4(fragPos, 1.0);
        // Se pasan las coordenadas del fragmento del mundo al espacio de la luz
        shadowCoord = biasMat * mainLightData.depthMVP * fragWorldPos;

	vec3 finalColor = vec3(0.0);

	// Iluminación sencilla con sombras
    	finalColor += shadeMainLight();

	outFragColor.rgb = finalColor;	
}