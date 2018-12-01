//#version 430 auto included by the preprocessor

// TODO List:
// seperate stuff into files
// Add to uniforms (lights, etc.)
// Benjamin noise stuff
// Everything


// ----------------------------------------------------- //
// Big globals
// ----------------------------------------------------- //

// should be included, but the minifier has issues with it
//precision mediump float;

layout (location=0) uniform vec4 fpar[4];
layout (location=0) out vec4 color;
in vec2 p;

vec2 resolution = vec2(fpar[0].y, fpar[0].z); // TODO: get this as a uniform

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

struct Material {
	vec3 diffuseColor;
	float specular;
	float shininess;
};

struct raymarchResult
{
	vec3 position;
	vec3 normal;
	bool hit;
	Material material;
};

// ----------------------------------------------------- //
// SDF functions
// ----------------------------------------------------- //

// copy-pasted from IQ
float opSmoothUnion( float d1, float d2, float k )
{
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float sphereSDF(in vec3 v, in vec3 center, in float radius)
{
	return length(v - center) - radius;
}

float sdPlane( vec3 p, vec4 n ) {
  // n must be normalized
  return dot(p,n.xyz) + n.w + sin(p.x/10.0)*3.0 + sin(p.z/10.0)*3.0;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf
}

float worldSDF(in vec3 v)
{
	float sdf_agg = 0.0;

	float plane = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));

	float blob = sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25 + sin(v.x) * sin(v.y) * sin(v.z)*0.1; // blobs on this sphere

	vec3 q = v.xyz;

	v.xz = mod(v.xz, vec2(20.0, 20.0)) - vec2(10.0, 10.0);

	float sphere1 = sphereSDF(v, vec3(-0.0,10 - sin(q.x/10.0)*3.0 - sin(q.z/10.0),0), 4.0);

	float sphere2 = sphereSDF(v, vec3(-5.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 0.0), 2.0);
	float sphere3 = sphereSDF(v, vec3(0.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 5.0), 2.0);
	float sphere4 = sphereSDF(v, vec3(0.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, -5.0), 2.0);
	float sphere5 = sphereSDF(v, vec3(5.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 0.0), 2.0);

	float c1 = opSmoothUnion(sphere1, sphere2, sin(TIME)*2.0+2.0); // unionize the spheres to make a... pinecone
	float c2 = opSmoothUnion(sphere3, sphere4, sin(TIME)*2.0+2.0);
	sdf_agg = opSmoothUnion(c1, c2, sin(TIME)*2.0+2.0);
	sdf_agg = opSmoothUnion(sphere5, sdf_agg, sin(TIME)*2.0+2.0);

	sdf_agg = opSmoothUnion(sdf_agg, plane, 3.0);

	//sdf_agg = max(sdf_agg, -sdBox(v, vec3(4.0, 10.0, 4.0)));
	sdf_agg = opSmoothUnion(sdf_agg, sdBox(v, vec3(1.0, 10.0, 1.0)), 3.5);

	return sdf_agg;
}

// ----------------------------------------------------- //
// Lights
// ----------------------------------------------------- //



// ----------------------------------------------------- //
// Rendering
// ----------------------------------------------------- //

// Tetrahedron technique from http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcWorldNormal(in vec3 p)
{
    const float h = 0.0001;
    const vec2 k = vec2(1.0,-1.0);
    return normalize(k.xyy*worldSDF(p + k.xyy*h) +
                     k.yyx*worldSDF(p + k.yyx*h) +
                     k.yxy*worldSDF(p + k.yxy*h) +
                     k.xxx*worldSDF(p + k.xxx*h));
}

const float MIN_HIT_DIST = 0.001;
const float MAX_DIST = 1000.0;
const float SPEED_MULTIPLIER = 1.0;

// @param ray origin and ray direction
raymarchResult worldMarch(in vec3 ro, in vec3 rd, const int MAX_STEPS) {

	raymarchResult marched;

	marched.hit = true;
	for(int i = 0; i < MAX_STEPS; i++) {

		float samp = worldSDF(ro); // find SDF at current march position

		// If SDF is low enough, handle the collision.
		if(abs(samp) < MIN_HIT_DIST) {
			break;
		}
		if(length(ro) > MAX_DIST) {
			marched.hit = false;
			break;
		}
		// Step ray forwards by SDF
		ro += rd*samp*SPEED_MULTIPLIER;
	}
	marched.position = ro;
	marched.normal = calcWorldNormal(ro);

	return marched;
}

float worldShadow(in vec3 ro, in vec3 rd, float hardness, const int MAX_STEPS) {

	ro += rd*MIN_HIT_DIST*30.0; // Increasing the factor here decreases the chance of banding, but makes less accurate shadows.

	float light = 1.0;
	float dist = 0.0;
	float psamp = 1e20;

	for(int i = 0; i < MAX_STEPS; i++) {
		float samp = worldSDF(ro);

		float y = samp*samp/(2.0*psamp);
		float d = sqrt(samp*samp-y*y);

		light = min(light, hardness*d/max(0.0, dist-y));

		if(samp < MIN_HIT_DIST) {
			return 0.0;
		}
		if(length(ro) > MAX_DIST) {
			break;
		}

		ro += rd*samp;
		dist += samp;
		psamp = samp;
	}
	return light;
}

vec3 render3(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 200);
	if(!cameraCast.hit) return vec3(0.0); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;

	vec3 lightDir = normalize(vec3(0.9, -1.0, 0.2));
	diffuse = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 20.0,100))*diffuse + vec3(0.1);

	vec3 lightDir2 = normalize(vec3(-0.9, -0.4, 0.6));
	vec3 diffuse2 = vec3(1.0, 1.0, 0.1)*max(vec3(0.0), dot(-lightDir2, cameraCast.normal))*0.8;
	cool += vec3(worldShadow(cameraCast.position, -lightDir2, 200.0, 100))*diffuse2;

	return cool;
}

vec3 render2(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 200);
	if(!cameraCast.hit) return vec3(0.0); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;

	vec3 lightDir = normalize(vec3(0.9, -1.0, 0.2));
	diffuse = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 20.0, 100))*diffuse + vec3(0.1);

	vec3 lightDir2 = normalize(vec3(-0.9, -0.4, 0.6));
	vec3 diffuse2 = vec3(1.0, 1.0, 0.1)*max(vec3(0.0), dot(-lightDir2, cameraCast.normal))*0.8;
	cool += vec3(worldShadow(cameraCast.position, -lightDir2, 200.0, 100))*diffuse2;

	cool += render3(cameraCast.position + cameraCast.normal*0.01, reflect(rd, cameraCast.normal))*0.8*vec3(0.7, 0.9, 1.0);

	return cool;
}

vec3 render(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 200);
	if(!cameraCast.hit) return vec3(0.0); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;

	vec3 lightDir = normalize(vec3(0.9, -1.0, 0.2));
	diffuse = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 20.0, 200))*diffuse + vec3(0.1);

	vec3 lightDir2 = normalize(vec3(-0.9, -0.4, 0.6));
	vec3 diffuse2 = vec3(1.0, 1.0, 0.1)*max(vec3(0.0), dot(-lightDir2, cameraCast.normal))*0.8;
	cool += vec3(worldShadow(cameraCast.position, -lightDir2, 200.0, 200))*diffuse2;

	cool += render2(cameraCast.position + cameraCast.normal*0.01, reflect(rd, cameraCast.normal))*0.8*vec3(0.7, 0.9, 1.0)*(1.0-dot(cameraCast.normal, normalize(ro - cameraCast.position)));

	return cool;
}

Camera getCam()
{
	Camera cam;
	vec3 lookAt = vec3(0, -12, 0);

	cam.position = vec3(cos(TIME*0.1)*100.0,15+ 3.0, sin(TIME*0.1)*100.0);

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

	color = vec4(render(cam1.position, cam1.rayDir), 1.0);
}
