const uvec3 SURFEL_GRID_DIMENSIONS = uvec3(256, 128, 128);
const uint SURFEL_TABLE_SIZE = SURFEL_GRID_DIMENSIONS.x * SURFEL_GRID_DIMENSIONS.y * SURFEL_GRID_DIMENSIONS.z; 
const float SURFEL_MAX_RADIUS = 12.5;
const float CELL_LENGTH = 30;
const uint SURFEL_CAPACITY = 100000;
const float SURFEL_TARGET_COVERAGE = 0.2;
const uint MAX_RAYS_PER_SURFEL = 2500;
const uint NUM_RAYS = 50;
const uint RAYS_LENGTH = 1500;
const float INDIRECT_DIFFUSE_ILLUMINATION_WEIGHT = 1.0;
const float ANGULAR_INDIRECT_DIFFUSE_MIN_FACTOR = 0.025;

#define PI 3.14159265358979323846
#define SQRT_PI 1.772453851

const uint SURFEL_CELL_LIMIT = 30;

struct Surfel
{
	vec3 position;
    	float radius;
	vec3 normal;
    	int generatedRays;
	vec3 color;
	float pad1;
	vec3 direct_radiance;
    	float padding2;
    	vec3 indirect_radiance;
    	float padding3;
};

ivec3 surfel_cell(vec3 position){
	return ivec3(floor((position) / CELL_LENGTH) + SURFEL_GRID_DIMENSIONS / 2);
}

bool surfel_cellValid(ivec3 cell){
	if (cell.x < 0 || cell.x >= SURFEL_GRID_DIMENSIONS.x)
		return false;
	if (cell.y < 0 || cell.y >= SURFEL_GRID_DIMENSIONS.y)
		return false;
	if (cell.z < 0 || cell.z >= SURFEL_GRID_DIMENSIONS.z)
		return false;
	return true;
}

uint flatten3D(uvec3 coord, uvec3 dim)
{
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}

uint surfel_cellIndex(ivec3 cell)
{
	return flatten3D(uvec3(cell), SURFEL_GRID_DIMENSIONS);
}

const vec3 surfel_neighbour_offsets[27] = {
	vec3(-1, -1, -1),
	vec3(-1, -1, 0),
	vec3(-1, -1, 1),
	vec3(-1, 0, -1),
	vec3(-1, 0, 0),
	vec3(-1, 0, 1),
	vec3(-1, 1, -1),
	vec3(-1, 1, 0),
	vec3(-1, 1, 1),
	vec3(0, -1, -1),
	vec3(0, -1, 0),
	vec3(0, -1, 1),
	vec3(0, 0, -1),
	vec3(0, 0, 0),
	vec3(0, 0, 1),
	vec3(0, 1, -1),
	vec3(0, 1, 0),
	vec3(0, 1, 1),
	vec3(1, -1, -1),
	vec3(1, -1, 0),
	vec3(1, -1, 1),
	vec3(1, 0, -1),
	vec3(1, 0, 0),
	vec3(1, 0, 1),
	vec3(1, 1, -1),
	vec3(1, 1, 0),
	vec3(1, 1, 1),
};

const vec3 surfel_direct_neighbour_offsets[7] = {
	vec3(0, 0, 0),
	vec3(-1, 0, 0),
	vec3(1, 0, 0),
	vec3(0, -1, 0),
	vec3(0, 1, 0),
	vec3(0, 0, -1),
	vec3(0, 0, 1)
};

float distanceSquared(vec3 a, vec3 b) {
    vec3 d = a - b;
    return dot(d, d);
}

bool surfel_cellIntersects(Surfel surfel, ivec3 cell)
{
    if (!surfel_cellValid(cell)) {
        return false;
    }

    const float cellSize = CELL_LENGTH;
    // Reconstruimos el AABB en espacio mundo desde la celda
    vec3 cellMin = (vec3(cell) - vec3(SURFEL_GRID_DIMENSIONS) / 2.0) * cellSize;
    vec3 cellMax = cellMin + vec3(cellSize);

    // Punto más cercano de la celda al surfel
    vec3 closestPoint = clamp(surfel.position, cellMin, cellMax);

    // Si está a menos de un radio, hay intersección
    float distSquared = dot(surfel.position - closestPoint, surfel.position - closestPoint);
    return distSquared < (SURFEL_MAX_RADIUS * SURFEL_MAX_RADIUS);
}

uint hash_uint(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
