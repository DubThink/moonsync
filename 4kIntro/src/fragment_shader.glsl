#version 430

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
in vec2 p;

// #include noise1.glsl
#include util.glsl

void main()
{
	// float t;
	// t = fpar[0].x * 1.0;
 //    vec2 r = vec2(1920.0,1080.0),
 //    o = gl_FragCoord.xy - r/2.;
 //    o = vec2(length(o) / r.y - .3, atan(o.y,o.x));
 //    vec4 s = 0.07*cos(1.5*vec4(0,1,2,3) + t + o.y + sin(o.y) * cos(t)),
 //    e = s.yzwx,
 //    f = max(o.x-s,e-o.x);
	// color = dot(clamp(f*r.y,0.,1.), 72.*(s-e)) * (s-.1) + f;
	vec2 st = gl_FragCoord.xy/vec2(1280,720);
    // Scale the coordinate system to see
    // some noise in action
    vec2 pos = vec2(st*20.0);

    // Use the noise function
    // float m = cellular(pos*.5).x;
    // float n = noise(pos)*.1+.05;
    // float v = m<n?n-m:n;
    // vec2 m = cellular(pos*.5);
    // m.x=m.x+fpar[0].x;


    color = vec4(greenish()*st.x, 1.0);
}
