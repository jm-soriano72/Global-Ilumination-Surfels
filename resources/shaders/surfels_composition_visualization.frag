#version 450

layout (binding = 10) uniform sampler2D surfelsVisualizationMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;


void main() 
{
	vec3 finalColor = texture(surfelsVisualizationMap, inUV).rgb;
	outFragColor.rgb = finalColor;	
}