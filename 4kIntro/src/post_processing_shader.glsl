//#version 430 auto included by the preprocessor

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
uniform sampler2D inputTexture;
in vec2 p;

#define time fpar[0].x;

void main()
{
	vec2 c = 0.5 * p + 0.5;
	color = texture(inputTexture, c);
}
