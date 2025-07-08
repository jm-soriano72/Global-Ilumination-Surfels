#version 450

layout (binding = 0) uniform sampler2D samplerposition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (binding = 3) uniform sampler2D samplerSSAOBlur;


layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);
	vec4 albedo = texture(samplerAlbedo, inUV);
	 
	float ssao = texture(samplerSSAOBlur, inUV).r;

	vec3 lightPos = vec3(0.0);
	vec3 L = normalize(lightPos - fragPos);
	float NdotL = max(0.0, dot(normal, L));

	vec3 baseColor = albedo.rgb * NdotL;
	outFragColor.rgb = ssao.rrr;
	outFragColor.rgb *= baseColor;
			
}