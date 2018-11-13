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
#include noise1.glsl
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

float terrain(vec2 p){
	return noise(p*0.05125)*5+noise(p*0.0125125)*20+noise(p*0.0025)*100;//sin(p.x/10.0)*3.0 + sin(p.x/100.0)*30.0 + sin(p.y/10.0)*3.0 + sin(p.y/100.0)*30.0;
}
// copy-pasted from IQ
float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float opSmoothSubtraction( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); }

float sphereSDF(in vec3 v, in vec3 center, in float radius)
{
	return length(v - center) - radius;
}

float sdPlane( vec3 p, vec4 n ) {
  // n must be normalized
  return dot(p,n.xyz) + n.w + terrain(p.xz);
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf
}

float sdTree(in vec3 v, in vec3 p){
	float sphere1 = sphereSDF(v, vec3(-0.0,10 - sin(p.x/10.0)*3.0 - sin(p.z/10.0),0), 4.0);

	float sphere2 = sphereSDF(v, vec3(-5.0, 10.0 - sin(p.x/10.0)*3.0 - sin(p.z/10.0)*3.0, 0.0), 2.0);
	float sphere3 = sphereSDF(v, vec3(0.0, 10.0 - sin(p.x/10.0)*3.0 - sin(p.z/10.0)*3.0, 5.0), 2.0);
	float sphere4 = sphereSDF(v, vec3(0.0, 10.0 - sin(p.x/10.0)*3.0 - sin(p.z/10.0)*3.0, -5.0), 2.0);
	float sphere5 = sphereSDF(v, vec3(5.0, 10.0 - sin(p.x/10.0)*3.0 - sin(p.z/10.0)*3.0, 0.0), 2.0);
	float sdf_agg = 0.0;

	float c1 = opSmoothUnion(sphere1, sphere2, sin(TIME)*2.0+2.0); // unionize the spheres to make a... pinecone
	float c2 = opSmoothUnion(sphere3, sphere4, sin(TIME)*2.0+2.0);
	sdf_agg = opSmoothUnion(c1, c2, sin(TIME)*2.0+2.0);
	sdf_agg = opSmoothUnion(sphere5, sdf_agg, sin(TIME)*2.0+2.0);
	sdf_agg = opSmoothUnion(sdf_agg, sdBox(p, vec3(1.0, 10.0, 1.0)), 1.5);
	return sdf_agg;
}
struct debugworldsdf
{
	float v;
	float d;
	vec3 cell;
};
debugworldsdf worldSDF(in vec3 v)
{
	debugworldsdf ret;
	float sdf_agg = 10000.0;

	float plane = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));
	vec2 seed=vec2(123,5);
	vec3 cell=vec3(0);
	vec3 vp=v;
	vp.xz=mod(v.xz,vec2(20));
	// vp.y=sin(v.x/10.0)*3.0 + sin(v.x/100.0)*30.0 + sin(v.z/10.0)*3.0 + sin(v.z/100.0)*30.0;
	cell.xz=v.xz-vp.xz;
	cell/=20;
	ret.cell=cell;
	vec3 cell_offset;
	vec3 dcell=vec3(0);
	for(int i=0;i<9;i++){
		dcell.xz=vec2(i%3-1,floor(i/3)-1);
		cell_offset=random3(vec3(cell.xz+dcell.xz+seed,0) ) * 20;
		cell_offset.y-=terrain(v.xz);
		sdf_agg = opSmoothUnion(sdf_agg,sphereSDF(vp-dcell*20,cell_offset,10*noise((cell.xz+dcell.xz)*0.1)+1.5),0.8 );
	}
	// float blob = sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25 + sin(v.x) * sin(v.y) * sin(v.z); // blobs on this sphere

	// // ray in cell space
	// vec3 q = v.xyz;
	//
	// q.xz = mod(v.xz, vec2(20.0, 20.0));
	// vec3 cell = v-q;
	// q.xz -= vec2(10.0, 10.0);
	//
	// vec3 cell_offset;
	// //200*floor(.005*mod(v.xz, vec2(20.0, 20.0)))
	// cell_offset.xz=6*random2(floor(cell/20).xz);
	// sdf_agg=sdTree(q+cell_offset,v+cell_offset);
	// for(int i=0;i<9;i++){
	// 	sdf_agg=sdTree(q,v);
	// 	cell_offset.xz=12*random2(floor(cell/20).xz-vec2(i%3-1,floor(i/3)-1))-vec2(i%3-1,floor(i/3)-1)*20;
	// 	sdf_agg=min(sdf_agg,sdTree(q+cell_offset,v+cell_offset));
	// }
	sdf_agg = opSmoothSubtraction(sdf_agg, plane, 3.0);
	ret.v=sdf_agg;
	return ret;
}

// ----------------------------------------------------- //
// Rendering
// ----------------------------------------------------- //

const int MAX_STEPS = 200;
const float MIN_HIT_DIST = 0.001;
const float MAX_DIST = 1000.0;

// @param ray origin and ray direction
raymarchResult worldMarch(in vec3 ro, in vec3 rd) {

	raymarchResult marched;
	marched.diffuse_color = vec3(0.5); // "sky" color

	for(int i = 0; i < MAX_STEPS; i++) {

		debugworldsdf dsamp = worldSDF(ro); // find SDF at current march position
		float samp = dsamp.v;
		// If SDF is low enough, handle the collision.
		// if(samp < MIN_HIT_DIST) {
		if(abs(samp) < MIN_HIT_DIST) {
			//marched.diffuse_color = vec3(float(i)/float(MAX_STEPS));
			//floor(.5*mod(ro*2,vec3(4)));
			marched.diffuse_color = vec3(noise(vec3(ro.xyz))*0.125+noise(vec3(ro.xyz*0.25))*0.25+noise(vec3(ro.xyz*0.05125))*0.5);//1.0, smoothstep(0.25,0.3,distance(vec2(0.5,0.5),fract(ro.xz))), 1.0);
			// if((mod(ro.x,40.0)>20)^^(mod(ro.z,40.0)>20))
				// marched.diffuse_color.r += .3;
			// marched.diffuse_color.g=min(1,dsamp.cell.z);
			break; // This, uhh, "wobbles" everything when it's active, and I don't know what to do about it. Working on that.
					 // I tried putting the whole if() after the ro incrementation, but it didn't do anything.
		}
		if(length(ro) > MAX_DIST) {
			marched.diffuse_color = vec3(normalize(ro));

			break;
		}

		// Step ray forwards by SDF
		//ro += (sin(ro)*abs(sin(TIME)/3.0) + rd)*samp; // make the rays go wiggly here
		ro += rd*samp;
	}
	marched.position = ro;

	return marched;
}

// Tetrahedron technique from http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcWorldNormal(in vec3 p)
{
    const float h = 0.0001;
    const vec2 k = vec2(1,-1);
    return normalize(k.xyy*worldSDF(p + k.xyy*h).v +
                     k.yyx*worldSDF(p + k.yyx*h).v +
                     k.yxy*worldSDF(p + k.yxy*h).v +
                     k.xxx*worldSDF(p + k.xxx*h).v);
}

Camera getCam()
{
	Camera cam;
	vec3 lookAt = fpar[2].xyz;

	cam.position = fpar[1].xyz;
	// cam.position.xz+=vec2(10);
	// figure out camera space from position and lookAt
	cam.up = vec3(0, 1, 0);
	cam.forwards = normalize(lookAt - cam.position);
	cam.left = cross(cam.forwards, cam.up);
	cam.up = cross(cam.left, cam.forwards);
	// cam.forwards*=2;
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
	// color += vec4(testMarch.diffuse_color*max(0, dot(norm, normalize(vec3(-0.9, 1.0, 0.2))))*vec3(vec3(abs(sin(TIME)), abs(sin(TIME+1)), abs(sin(TIME+2)))), 0.0);
	vec3 ldir = normalize(vec3(1, -1, 1));
	vec3 reflected = reflect(-ldir, norm);
	float spec = pow(max(0.0, dot(reflected, cam1.rayDir)), 200.0);
	color.xyz += vec3(spec)*testMarch.diffuse_color;
	//color = vec4(testMarch.diffuse_color, 1.0);
}
