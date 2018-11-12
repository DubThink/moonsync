
// TODO List: 
// seperate stuff into files
// Add to uniforms (lights, etc.)
// Benjamin noise stuff
// Everything

#version 430

// ----------------------------------------------------- //
// Big globals
// ----------------------------------------------------- //

precision mediump float;

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
in vec2 p;

vec2 resolution = vec2(1280, 720); // TODO: get this as a uniform

#define TIME fpar[0].x

// ----------------------------------------------------- //
// Structs and stuff
// ----------------------------------------------------- //

struct Camera
{
	vec3 position;
	
	// Camera space stuff
	vec3 forwards;
	vec3 left; // remove left/up to save space in future
	vec3 up;
	
	vec3 rayDir;
};

struct raymarchResult
{
	vec3 position;
	vec3 diffuse_color;
};

// ----------------------------------------------------- //
// SDF functions
// ----------------------------------------------------- //

// copy-pasted from IQ
float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float sphereSDF(in vec3 v, in vec3 center, in float radius)
{
	return length(v - center) - radius;
}

float worldSDF(in vec3 v)
{
	float sdf_agg = 0.0;

	float sphere1 = sphereSDF(v, vec3(2.0,0,0), 5.0);
	sphere1 += sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25; // blobs on this sphere

	float sphere2 = sphereSDF(v, vec3(-4.0, 0.0, 0.0), 3.0);

	sdf_agg = opSmoothUnion(sphere1, sphere2, sin(TIME)*2.0+2.0); // unionize the spheres to make a... pinecone

	sdf_agg = max(sdf_agg, -sphereSDF(cos(v), vec3(1.0, 3.0, 2.0), 4.0));

	//sdf_agg = abs(cos(sin(v.x)) + cos(v.y) + cos(v.z)) - 0.5;
	//sdf_agg = max(sdf_agg, sphereSDF(v, vec3(0.0, 0.0, 0.0), 8.0));

	return sdf_agg;
}

// ----------------------------------------------------- //
// Rendering
// ----------------------------------------------------- //

const int MAX_STEPS = 500;
const float MIN_HIT_DIST = 0.001;

// @param ray origin and ray direction
raymarchResult worldMarch(in vec3 ro, in vec3 rd) {
	
	raymarchResult marched;
	marched.diffuse_color = vec3(0,0,0); // "sky" color
	
	for(int i = 0; i < MAX_STEPS; i++) {
		
		float samp = worldSDF(ro); // find SDF at current march position

		// If SDF is low enough, handle the collision.
		if(samp < MIN_HIT_DIST) {
			marched.diffuse_color = vec3(0.1) + vec3(float(i)/float(MAX_STEPS)*4.0);
			break; // This, uhh, "wobbles" everything when it's active, and I don't know what to do about it. Working on that.
					 // I tried putting the whole if() after the ro incrementation, but it didn't do anything.
		}

		// Step ray forwards by SDF
		ro += (sin(ro)*abs(sin(TIME)/3.0) + rd)*samp; // make the rays go wiggly here
	}
	marched.position = ro;

	return marched;
};

// Tetrahedron technique from http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcWorldNormal(in vec3 p)
{
    const float h = 0.0001;
    const vec2 k = vec2(1,-1);
    return normalize(k.xyy*worldSDF(p + k.xyy*h) +
                     k.yyx*worldSDF(p + k.yyx*h) +
                     k.yxy*worldSDF(p + k.yxy*h) +
                     k.xxx*worldSDF(p + k.xxx*h));
}

Camera getCam()
{
	Camera cam;
	vec3 lookAt = vec3(0, 0, 0);
	
	cam.position = vec3(cos(TIME)*10.0, 3.0, sin(TIME)*10.0);
	
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
	raymarchResult testMarch = worldMarch(cam1.position, cam1.rayDir);
	vec3 norm = calcWorldNormal(testMarch.position);

	color += vec4(testMarch.diffuse_color*max(0, dot(norm, normalize(vec3(1.0, 1.0, 0.0))))*vec3(1.0, 1.0, 1.0), 1.0);
	color += vec4(testMarch.diffuse_color*max(0, dot(norm, normalize(vec3(0.1, -1.0, 0.2))))*vec3(0.3, 0.3, 0.5), 0.0);
	color += vec4(testMarch.diffuse_color*max(0, dot(norm, normalize(vec3(-0.9, 1.0, 0.2))))*vec3(vec3(abs(sin(TIME)), abs(sin(TIME+1)), abs(sin(TIME+2)))), 0.0);
	//color = vec4(testMarch.diffuse_color, 1.0);
}