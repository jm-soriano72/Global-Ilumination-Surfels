#version 450
#extension GL_EXT_scalar_block_layout : enable
#include "surfelsData.glsl"

layout(push_constant) uniform PushConstants {
    float width;
    float height;
} windowSize;

const float EPSILON = 1e-4;
const int REGION_RADIUS = 2;  // Región 5x5x5
const float SIGMA_BASE = 5.0;
const float SIGMA_SCALE = 6.0;
const float MIN_WEIGHT_THRESHOLD = 1e-5;

layout (binding = 1) readonly buffer SurfelBuffer { 
    Surfel surfels[];             
} surfels;
layout (binding = 2) readonly buffer GridBuffer { 
    uint cells[SURFEL_TABLE_SIZE]; 
} gridCells;
layout (binding = 3) readonly buffer CellBuffer { 
    uint indexSurfels[SURFEL_TABLE_SIZE * SURFEL_CELL_LIMIT]; 
} surfelCells;
layout (binding = 4) uniform sampler2D positionTexture;
layout (binding = 5) uniform sampler2D normalTexture;
layout (binding = 6) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
    vec2 nearFarPlanes;
    vec2 padding0;
    vec4 frame;
    vec3 position;
    float padding1;
} cameraData;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

float gaussianWeight(float d2, float sigma) {
    float invTwoSigma2 = 1.0 / (2.0 * sigma * sigma + 1e-6);
    return exp(-d2 * invTwoSigma2);
}

void main() 
{
    // Se recuperan los parámetros del G-Buffer
    vec3 fragPositionCamera = texture(positionTexture, inUV).xyz;
    vec3 fragNormalCamera = normalize(texture(normalTexture, inUV).xyz * 2.0 - 1.0);
    // Se pasan a espacio global
    vec3 fragWorldPosition = (inverse(cameraData.view) * vec4(fragPositionCamera, 1.0)).xyz;
    vec3 fragWorldNormal = normalize((inverse(cameraData.view) * vec4(fragNormalCamera, 0.0)).xyz);

    // Variables para almacenar la radiancia total y el peso total, para posteriormente poder normalizar
    vec3 totalRadiance = vec3(0.0);
    float totalWeight = 0.0;

    // Se calcula en qué celda está el fragmento y si se encuentra dentro del grid
    ivec3 baseCell = surfel_cell(fragWorldPosition);
    if (!surfel_cellValid(baseCell)) {
        outColor = vec4(0.0);
        return;
    }

    // Se recorre una región 5x5x5 de celdas respecto a la casilla en la que se encuentra en fragmento actual
    for (int dx = -REGION_RADIUS; dx <= REGION_RADIUS; ++dx) {
    for (int dy = -REGION_RADIUS; dy <= REGION_RADIUS; ++dy) {
    for (int dz = -REGION_RADIUS; dz <= REGION_RADIUS; ++dz) {
        ivec3 c = baseCell + ivec3(dx, dy, dz);
        if (!surfel_cellValid(c)) continue;

        uint cellIdx = surfel_cellIndex(c);
        uint count = gridCells.cells[cellIdx];

        for (int si = 0; si < count; ++si) {
            uint surfelIndex = surfelCells.indexSurfels[cellIdx * SURFEL_CELL_LIMIT + si];
            Surfel s = surfels.surfels[surfelIndex];

            float dist2 = distance(fragWorldPosition, s.position);
            dist2 *= dist2;

	    float sqrt_surfelArea = SQRT_PI * s.radius;

            float sigma = SIGMA_BASE + sqrt_surfelArea * SIGMA_SCALE;
            float gauss = gaussianWeight(dist2, sigma);

            float nDot = dot(fragWorldNormal, s.normal);
            float angular = max(nDot, 0.1);

            float weight = gauss * angular;
            if (weight < MIN_WEIGHT_THRESHOLD) continue;

            totalRadiance += s.direct_radiance * weight;
            totalWeight += weight;
        }
    }}}

    vec3 finalColor = totalRadiance / totalWeight;
    outColor = vec4(finalColor, 1.0);
}