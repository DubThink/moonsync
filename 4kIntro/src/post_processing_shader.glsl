#version 430

layout (location=0) out vec4 color;
uniform sampler2D inputTexture;
in vec2 p;

void main()
{
	vec2 coords = 0.5 * p + 0.5;
	if(coords.x<0.5)
	color = vec4(coords.x,0,coords.y,0);
	else
	color = texture(inputTexture, coords);
}