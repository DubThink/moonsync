//#version 430 auto included by the preprocessor

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
uniform sampler2D inputTexture;
in vec2 p;

#define time fpar[0].x;
vec2 resolution = vec2(fpar[0].y, fpar[0].z); // TODO: get this as a uniform

float bounds(in vec2 p, in vec2 mi,in vec2 ma){

	if(p.x<ma.x&&p.y<ma.y&&p.x>mi.x&&p.y>mi.y)return 1;
	else return 0;
}

float LW(in vec2 p){
	return bounds(p,vec2(.47,.52),vec2(.53))+
	bounds(p,vec2(.47,.47),vec2(.48,.52))+
	bounds(p,vec2(.495,.49),vec2(.505,.52))+
	bounds(p,vec2(.52,.47),vec2(.53,.52));
}

float LI(in vec2 p){
	return bounds(p,vec2(.495,.47),vec2(.505,.53))+
	bounds(p,vec2(.48,.47),vec2(.52,.48))+
	bounds(p,vec2(.48,.52),vec2(.52,.53));
}

float LN(in vec2 p){
	float f = bounds(p,vec2(.48,.47),vec2(.49,.53))+
	bounds(p,vec2(.51,.47),vec2(.52,.53));
	p.x+=(p.y-.50)/2;
	f+=bounds(p,vec2(.495,.47),vec2(.505,.53));
	return f;
}


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
	color+=val/5;//sqrt(val*3)/3;

	if(abs(distance(gl_FragCoord.xy,resolution.xy/2)-6)<1){
		color = vec4(.3,1.,.15, 1.0);
	}
	float kills=fpar[0].w;
	if(c.x<kills*0.04&&mod(c.x,0.04)>0.02&&c.y>0.97){
		color = vec4(.3,1.,.15, 1.0);
	}
	if(kills>=25){
		float text=0;
		c=vec2(1)-c;
		text=LW(c-vec2(.1,0))+LI(c)+LN(c+vec2(.1,0));
		text = min(1,text);
		if(text>0)
			color += vec4(.2,1,.2,1.)*(sin(fpar[0].x*5)/2+.5);
	}
}
