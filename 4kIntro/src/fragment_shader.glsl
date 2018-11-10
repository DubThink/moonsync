#version 430

precision mediump float;

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
in vec2 p;

#define time fpar[0].x
vec2 resolution = vec2(1920, 1080);

struct Camera
{
	vec3 position;
	
	// Camera space stuff
	vec3 forwards;
	vec3 left; // remove left/up to save space in future
	vec3 up;
	
	vec3 rayDir;
};

Camera getCam()
{
	Camera cam;
	vec3 lookAt = vec3(0, 0, 0);
	
	cam.position = vec3(cos(time)*10.0, 3.0, sin(time)*10.0);
	
	// figure out camera space from position and lookAt
	cam.up = vec3(0, 1, 0);
	cam.forwards = normalize(lookAt - cam.position);
	cam.left = cross(cam.forwards, cam.up);
	cam.up = cross(cam.left, cam.forwards);
	
	// find view ray - fustrum intersection for this pixel
	vec3 fustrumFront = cam.position + cam.forwards;
	vec2 screenSpace = 2.0*gl_FragCoord.xy/resolution.xy - 1.0;
	float aspect = resolution.x/resolution.y;
	vec3 fustrumIntersect = fustrumFront + screenSpace.x*cam.left*aspect + screenSpace.y*cam.up;
	
	// direction to march in
	cam.rayDir = normalize(fustrumIntersect-cam.position);
	
	return cam;
}
	
void main(void) {
	Camera cam1 = getCam();

	vec3 poos = cam1.rayDir;

	color = vec4(sin(poos.x*50.0), sin(poos.y*50.0), sin(poos.z*50.0), 1.0);
}