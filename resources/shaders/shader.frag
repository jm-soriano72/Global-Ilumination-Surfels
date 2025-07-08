#version 450
#extension GL_EXT_nonuniform_qualifier : require
#define NUM_LIGHTS 4

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in flat int fragMaterial;
layout(location = 4) in vec3 inViewVec;
layout(location = 5) in vec3 inLightVec;
layout(location = 6) in vec4 shadowCoord; // Coordenadas en espacio de luz, para shadow mapping

layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout(binding = 1) uniform sampler2D[] texSamplers;

layout(push_constant) uniform PushConstants {
    vec3 cameraPosition;
    uint enablePCF;
} push;

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

const vec3 ambientIntensity = vec3(0.01); // Luz ambiental global

// Parámetros de atenuación
float d0 = 1.0; // Distancia a la que se conoce la intensidad de la luz
float d_max = 100000.0; // Distancia máxima a la que llega la luz

// Propiedades del objeto
vec3 Ka; // Color ambiental del material
vec3 Kd; // Componente difusa del material
vec3 Ks; // Componente especular del material
float rugosityAlpha = 3.0;

// Cálculo del shadow mapping
layout(binding = 4) uniform sampler2D shadowMap; // Textura con la profundidad de la primera pasada

// Declaración de funciones
float shadowCalculation();
float shadowPCFCalculation();
float fDist(float d0, float d, float d_max);
vec3 shadePuntual(Light l);
vec3 shadeDireccional();
vec3 shadeMainLight();

void main() {

    // Se calculan las componentes del material
    // Por el momento, hay 3 canales de textura por material, por lo tanto, se calcula el índice de textura en base a esto
    uint materialId = fragMaterial * 3;

    // El mapa de transparencia se encuentra en la segunda posición
    // Se obtiene el alfa del canal r de la textura
    float alpha = texture(texSamplers[nonuniformEXT(materialId + 1)], fragTexCoord).r;
    // En caso de que el alfa sea nulo, se descarta el fragmento para que sea transparente
    if (alpha < 0.1) discard;

    // Vector donde se va a acumular la iluminación
    vec3 finalColor = vec3(0.0);
    // Se calculan las componentes del material 
    Ka = texture(texSamplers[nonuniformEXT(materialId)], fragTexCoord).rgb;
    Kd = texture(texSamplers[nonuniformEXT(materialId)], fragTexCoord).rgb;
    // El mapa especular se encuentra en la tercera posición
    Ks = texture(texSamplers[nonuniformEXT(materialId + 2)], fragTexCoord).rgb;

    // Iluminación sencilla con sombras
    finalColor += shadeMainLight();

    // Iluminación con fuentes puntuales
    for (int i = 0; i < NUM_LIGHTS; i++) {
        Light light = sceneLights.lights[i];
        // Se interpretan todas las fuentes de luz como puntuales, por el momento
        finalColor += shadePuntual(light);       
    }

    outColor = vec4(finalColor, 1.0);
}

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

// Cálculo del factor de atenuación

float fDist (float d0, float d, float d_max) {
	
	float fdist;
	float fwin;

	fwin = pow(max(0,(1-pow((d/d_max),4))),2);
	fdist = pow((d0/d),2)*fwin;

	return fdist;

}

// Para aplicar la iluminación de Phong, la función de shade que la aplica se debe calcular en los fragmentos
// En los vértices solo se recalcula la normal y la posición en coordenadas de la cámara

vec3 shadePuntual (Light l) {

	vec3 c = vec3(0.0); // Color negro por defecto
        // Se pasa la posición de la luz a coordenadas de la cámara
        vec3 lightPositionCamera = (ubo.view * vec4(l.position,1.0)).xyz;
        // Se normaliza el vector normal
        vec3 N = normalize(fragNormal);

        // FUNCIÓN DE ATENUACIÓN //
	float light_distance = distance(lightPositionCamera, fragPosition); // Distancia entre el foco de luz y la posición del objeto 
	float atenuation = fDist(d0, light_distance, d_max); // Se calcula el factor de atenuación

	// PARTE DIFUSA //
	vec3 L = normalize(lightPositionCamera - fragPosition); // Se normaliza la dirección de la luz
	vec3 diffuse = l.intensity * Kd * dot(L,N); // Cálculo de la componente difusa
	c+= diffuse * atenuation;

        // PARTE ESPECULAR //
	vec3 V = normalize (-fragPosition); // El vector V es la dirección de la luz reflectada dependiendo del punto de vista, por lo que se toma como referencia el vector pos, que tiene dirección opuesta al objeto y por lo tanto se pasa negativa.
	vec3 R = normalize (reflect(-L,N)); // La función reflect, dado un rayo incidente y una normal, calcula el vector reflejado R perfecto, independientemente del punto de vista.
	float factor = clamp(dot(R,V), 0.0, 1.0);
	vec3 specular = l.intensity * Ks * pow(factor, rugosityAlpha);
	c+= specular * atenuation;

        c += ambientIntensity * Ka;

	c = clamp(c, 0.0, 1.0);

	return c;

}

vec3 shadeDireccional() {
        
        vec3 c = vec3(0.0); // Color negro por defecto
        // Se normaliza el vector normal
        vec3 N = normalize(fragNormal);

	// PARTE DIFUSA //
	vec3 L = normalize(ubo.view * vec4(sceneLights.mainLight.direction, 0.0)).xyz;
	vec3 diffuse = sceneLights.mainLight.intensity * Kd * dot(L,N); 

	// PARTE ESPECULAR // 
	vec3 V = normalize (-fragPosition); 
	vec3 R = normalize (reflect(-L,N)); 
	float factor = clamp(dot(R,V), 0.0, 1.0);
	vec3 specular = sceneLights.mainLight.intensity * Ks * pow(factor, rugosityAlpha); 

	c += (diffuse + specular);

	c = clamp(c,0.0,1.0);

	return c;
}

vec3 shadeMainLight() {

    vec3 c = vec3(0.0); // Color negro por defecto

    // PARTE DIFUSA //
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(inLightVec);
    vec3 diffuse = sceneLights.mainLight.intensity * Kd * max(dot(L,N), 0.0);
    c+= diffuse;

    // PARTE ESPECULAR // 
    vec3 V = normalize(inViewVec);
    vec3 R = normalize(-reflect(L,N));
    float factor = clamp(dot(R,V), 0.0, 1.0);
    vec3 specular = sceneLights.mainLight.intensity * Ks * pow(factor, rugosityAlpha);
    c+= specular;

    float shadowFactor = (push.enablePCF == 1) ? shadowPCFCalculation(shadowCoord/shadowCoord.w) : shadowCalculation(shadowCoord/shadowCoord.w, vec2(0.0));
    c *= shadowFactor; // Cálculo del factor de sombreado

    // PARTE AMBIENTAL //
    c += ambientIntensity * Ka; // El color va a depender del color ambiental del objeto, junto con la intensidad de la luz ambiental

    c = clamp(c, 0.0, 1.0);

    return c;

}

