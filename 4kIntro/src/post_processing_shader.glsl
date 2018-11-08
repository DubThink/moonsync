#version 430

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
uniform sampler2D inputTexture;
in vec2 p;

#include util.glsl
#define time fpar[0].x

void main()
{
	vec2 c = 0.5 * p + 0.5;
	vec2 q = c- vec2(0.5,0.5);
	vec3 col = greenish();//vec3(1.0,0.5,0.1);
	float r = 0.2 + 0.1*cos(atan(q.y,q.x)*10.0 + 20.0*q.x +sin(time));
	col *= smoothstep(r,r+.01, length(q) );
	// color = texture(inputTexture, c);
	color=vec4(col,1.0);
}
