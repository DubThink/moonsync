//#version 430 auto included by the preprocessor

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
uniform sampler2D inputTexture;
in vec2 p;

#define time fpar[0].x;
vec2 resolution = vec2(fpar[0].y, fpar[0].z); // TODO: get this as a uniform

void main()
{
	vec2 c = 0.5 * p + 0.5;
	color = max(vec4(0),texture(inputTexture, c));

	vec4 val;
	for(int i=-20;i<=20;i++){
		val+= max(vec4(0),texture(inputTexture,c+vec2(i/fpar[0].y,0)) - vec4(0.7)) * 0.2  * (1-abs(i*0.05));//-vec4(1)
	}
	for(int i=-20;i<=20;i++){
		val+= max(vec4(0),texture(inputTexture,c+vec2(0,i/fpar[0].z)) - vec4(0.7)) * 0.2  * (1-abs(i*0.05));//-vec4(1)
	}
	color+=val/3;//sqrt(val*3)/3;

	if(abs(distance(gl_FragCoord.xy,resolution.xy/2)-6)<1){
		color = vec4(.3,1.,.15, 1.0);
	}
}
