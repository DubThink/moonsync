#version 430

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
in vec2 p;


void main()
{
	float t;
	t = fpar[0].x * 1.0;
    vec2 r = vec2(1920.0,1080.0),
    o = gl_FragCoord.xy - r/2.;
    o = vec2(length(o) / r.y - .3, atan(o.y,o.x));    
    vec4 s = 0.07*cos(1.5*vec4(0,1,2,3) + t + o.y + sin(o.y) * cos(t)),
    e = s.yzwx, 
    f = max(o.x-s,e-o.x);
	color = dot(clamp(f*r.y,0.,1.), 72.*(s-e)) * (s-.1) + f;
}