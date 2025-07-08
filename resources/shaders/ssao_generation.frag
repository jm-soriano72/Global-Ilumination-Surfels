#version 450

layout (binding = 0) uniform sampler2D samplerPositionDepth;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D ssaoNoise;

layout (constant_id = 0) const int SSAO_KERNEL_SIZE = 64;
layout (constant_id = 1) const float SSAO_RADIUS = 37.5;

layout (binding = 3) uniform UBO 
{
	mat4 projection;
} ubo;

layout (binding = 4) uniform UBOSSAOKernel
{
	vec4 samples[SSAO_KERNEL_SIZE];
} uboSSAOKernel;

layout (location = 0) in vec2 inUV;

layout (location = 0) out float outFragColor;

void main() 
{
	// Se recuperan los parámetros del G-Buffer
        // Tanto la posición como las normales están en coordenadas de cámara
	vec3 fragPos = texture(samplerPositionDepth, inUV).rgb;
        // Las normales se guardaron normalizadas, con coordenadas [0,1], por lo que se vuelven a pasar a [-1,1]
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);

	// Se obtiene un vector aleatorio utilizando la textura de ruido generada
	ivec2 texDim = textureSize(samplerPositionDepth, 0); 
	ivec2 noiseDim = textureSize(ssaoNoise, 0);
	const vec2 noiseUV = vec2(float(texDim.x)/float(noiseDim.x), float(texDim.y)/(noiseDim.y)) * inUV;  
	vec3 randomVec = texture(ssaoNoise, noiseUV).xyz * 2.0 - 1.0;
	
	// Se proyecta el vector aleatorio sobre el plano perpendicular a la normal, obteniendo la tangente
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
        // Se calcula la bitangente como el producto vectorial de ambos
	vec3 bitangent = cross(tangent, normal);
        // La matriz TBN se construye para poder pasar al espacio local del fragmento
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Valor acumulado de la oclusión
	float occlusion = 0.0f;
	const float bias = 0.025f;
	for(int i = 0; i < SSAO_KERNEL_SIZE; i++)
	{
                // Se pasa el sample tomado al espacio del fragmento		
		vec3 samplePos = TBN * uboSSAOKernel.samples[i].xyz;
                // Se utiliza el radio para generar un hemisferio de muestras 
		samplePos = fragPos + samplePos * SSAO_RADIUS; 
		
		// Se proyecta la muestra, para que, utilizando sus coordenadas proyectadas, se pueda tomar su profundidad
                // de la textura
		vec4 offset = vec4(samplePos, 1.0f);
		offset = ubo.projection * offset; 
		offset.xyz /= offset.w; 
		offset.xyz = offset.xyz * 0.5f + 0.5f; 
		// Se obtiene la profundidad de la muestra
		float sampleDepth = texture(samplerPositionDepth, offset.xy).z; 
                // Con esto se evita que fragmentos que aparecen cercanos al ser proyectados participen en la oclusión
                // si están muy lejos en la escena
		float rangeCheck = smoothstep(0.0f, 1.0f, SSAO_RADIUS / abs(fragPos.z - sampleDepth));
                // Se realiza la comparación de ambas profundidades
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;           
	}
        // La salida de la oclusión se calcula como la media de todas las muestras
	occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
	
	outFragColor = occlusion;
}