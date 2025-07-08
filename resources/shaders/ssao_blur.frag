#version 450

layout (binding = 0) uniform sampler2D samplerSSAO;

layout (location = 0) in vec2 inUV;

layout (location = 0) out float outFragColor;

void main() 
{
        const float blurRange = 3, blurStep = 1;
	int n = 0;
	vec2 texelSize = 1.0 / vec2(textureSize(samplerSSAO, 0));
	float result = 0.0;
	for (float x = -blurRange; x <= blurRange; x+= blurStep) 
	{
		for (float y = -blurRange; y <= blurRange; y+= blurStep) 
		{
			vec2 offset = vec2(x, y) * texelSize;
			result += texture(samplerSSAO, inUV + offset).r;
			n++;
		}
	}
	outFragColor = result / (float(n));
}
