#version 450

layout(location = 0) in vec2  gs_UV;
layout(location = 1) in vec3  gs_Color;

layout(location = 0) out vec4 outColor;

void main() {
    // De [0,1]² a [-1,1]²
    vec2 uv = gs_UV * 2.0 - 1.0;
    // Fuera del círculo unitario ⇒ descarta
    if (dot(uv, uv) > 1.0) {
        discard;
    }
    outColor = vec4(gs_Color, 1.0);
}