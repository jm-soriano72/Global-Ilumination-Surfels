#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// Se toman como 
layout(location = 0) in VS_OUT {
    vec3 pos;
    vec3 normal;
    float radius;
    vec3 color;
} gs_in[];

// Atributos para pasar al shader de fragmentos
layout(location = 0) out vec2  gs_UV;
layout(location = 1) out vec3  gs_Color;

// Mismo uniform de cámara
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
    // Calcula dos vectores ortogonales al normal
    vec3 N = normalize(gs_in[0].normal);
    vec3 T = normalize(cross(abs(N.y) < 0.99 ? vec3(0,1,0) : vec3(1,0,0), N));
    vec3 B = cross(N, T);

    // Cuatro esquinas en el plano del surfel
    vec2 corners[4] = vec2[]( vec2(-1,-1), vec2(1,-1),
                             vec2(-1, 1), vec2(1, 1) );
    for (int i = 0; i < 4; ++i) {
        vec2 c = corners[i];
        // Offset en mundo
        vec3 worldPos = gs_in[0].pos + (T * c.x + B * c.y) * gs_in[0].radius;
        // Proyecta la esquina
        gl_Position = cameraData.projection * cameraData.view * vec4(worldPos, 1.0);
        // UV para recorte circular
        gs_UV    = (c + 1.0) * 0.5;  
        gs_Color = gs_in[0].color;
        EmitVertex();
    }
    EndPrimitive();
}