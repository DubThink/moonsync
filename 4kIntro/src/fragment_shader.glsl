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
vec3 light1dir=vec3(0.6, -1.0, -0.2);
vec3 light2dir=vec3(-0.2, -0.8, 0.2);


float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

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
  return dot(p,n.xyz) + n.w;// + sin(p.x/10.0)*3.0 + sin(p.z/10.0)*3.0;
}

float sdVerticalCapsule( vec3 p, float h, float r )
{
	p.y -= clamp( p.y, 0.0, h );
	return length( p ) - r;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf
}
float opSubtraction( float d1, float d2 ) { return max(-d1,d2); }

float opSmoothSubtraction( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); }

float sdCappedCylinder( vec3 p, vec2 h )
{
	vec2 d = abs(vec2(length(p.xz),p.y)) - h;
	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdRoundBox( vec3 p, vec3 b, float r )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0)) - r
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf
}

float worldSDF2(in vec3 v)
{
	float sdf_agg = 0.0;

	sdf_agg = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));

	return sdf_agg;
}

float worldSDF1(in vec3 v)
{
	float sdf_agg = 0.0;

	float plane = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));
	float playerModel = sphereSDF(v-vec3(0,10,0),vec3(0),1);//sdVerticalCapsule(v,2,1);
	playerModel = opSubtraction(
		min(sphereSDF(vec3(v.x,v.y-10,abs(v.z)),vec3(.6,.4,.4),.3),
		sphereSDF(vec3(v.x,v.y-10,v.z),vec3(.6,-.4,0),.4)),
		playerModel);
	playerModel = min(sphereSDF(vec3(v.x,v.y-10,abs(v.z)),vec3(.6,.4,.4),.1),playerModel)+1000;
	float box1 = sdBox(v, vec3(36.0, 22.0, 16.0));
	float box2 = sdBox(v, vec3(34.5, 35.0, 14.5));

	// -- Arches - lower floor
	float archesLowerFloor = min(sdCappedCylinder(vec3(v.y-6,v.x,mod(v.z+5,10)-5),vec2(4.5,36)),sdCappedCylinder(vec3(mod(v.x+5,10)-5,v.z,v.y-6),vec2(4.5,15)));
	archesLowerFloor = opSubtraction(archesLowerFloor, sdBox(v-vec3(0,8.5,0),vec3(36.0,2.5, 15.0)));
	float mainPillars = sdCappedCylinder(vec3(mod(v.x,10)-5,v.y,mod(v.z,10)-5),vec2(.4,22));
	archesLowerFloor = opSmoothUnion(archesLowerFloor,mainPillars,.5);

	vec3 q=v-vec3(0,11,0);
	float archesUpperFloor = min(sdCappedCylinder(vec3(q.y-6,q.x,mod(q.z+5,10)-5),vec2(4.5,36)),sdCappedCylinder(vec3(mod(q.x+5,10)-5,q.z,q.y-6),vec2(4.5,15)));
	archesUpperFloor = opSubtraction(archesUpperFloor, sdBox(q-vec3(0,10.5,0),vec3(36.0,3.5, 15.0)));

	float arches = opSmoothUnion(archesUpperFloor,archesLowerFloor,.5);
	arches=min(arches,sdBox(v+vec3(0,0.75,0),vec3(36.0,.5, 15.0)));
	arches = min(arches,min( sdBox(vec3(v.x-0,v.y-12,abs(v.z)-5), vec3(25.0, 1.,.25)),sdBox(vec3(abs(v.x)-25,v.y-12,v.z), vec3(.25, 1.,5))));
	sdf_agg = min(opSubtraction(sdBox(v,vec3(24,35,4.5)),min(arches,opSubtraction(box2,box1))), playerModel);
	// float blob = sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25 + sin(v.x) * sin(v.y) * sin(v.z)*0.1; // blobs on this sphere

	// vec3 q = v.xyz;

	// v.xz = mod(v.xz, vec2(20.0, 20.0)) - vec2(10.0, 10.0);

	// float sphere1 = sphereSDF(v, vec3(-0.0,10 - sin(q.x/10.0)*3.0 - sin(q.z/10.0),0), 4.0);
	//
	// float sphere2 = sphereSDF(v, vec3(-5.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 0.0), 2.0);
	// float sphere3 = sphereSDF(v, vec3(0.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 5.0), 2.0);
	// float sphere4 = sphereSDF(v, vec3(0.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, -5.0), 2.0);
	// float sphere5 = sphereSDF(v, vec3(5.0, 10.0 - sin(q.x/10.0)*3.0 - sin(q.z/10.0)*3.0, 0.0), 2.0);
	//
	// float c1 = opSmoothUnion(sphere1, sphere2, sin(TIME)*2.0+2.0); // unionize the spheres to make a... pinecone
	// float c2 = opSmoothUnion(sphere3, sphere4, sin(TIME)*2.0+2.0);
	// sdf_agg = opSmoothUnion(c1, c2, sin(TIME)*2.0+2.0);
	// sdf_agg = opSmoothUnion(sphere5, sdf_agg, sin(TIME)*2.0+2.0);

	// sdf_agg = opSmoothUnion(sdf_agg, plane, 3.0);

	//sdf_agg = max(sdf_agg, -sdBox(v, vec3(4.0, 10.0, 4.0)));
	// sdf_agg = opSmoothUnion(sdf_agg, sdBox(v, vec3(1.0, 10.0, 1.0)), 3.5);
	// sdf_agg=min(sdf_agg,worldSDF2(v));
	return sdf_agg;
}

float worldSDF(in vec3 v)
{

	return min(worldSDF1(v),worldSDF2(v));
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

const float MIN_HIT_DIST = 0.01;
const float MAX_DIST = 100.0;
const float SPEED_MULTIPLIER = 1.0;

// @param ray origin and ray direction
raymarchResult worldMarch(in vec3 ro, in vec3 rd, const int MAX_STEPS) {

	raymarchResult marched;
	// marched.material.diffuseColor=ro*vec3(.4,.4,1.);
	marched.hit = true;
	for(int i = 0; i < MAX_STEPS; i++) {

		float samp = worldSDF(ro); // find SDF at current march )
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
	if(worldSDF2(ro)<MIN_HIT_DIST){
		marched.material.shininess=2;
	}else {
		marched.material.shininess=0.2;
	}
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

vec3 skyColor(in vec3 v){
	float factor=abs(dot(v,vec3(0,1,0)));
	float cloud=1-max(0,noise(v*10)-0.3);
	return vec3(0.2+cloud,0.2+cloud,0.6+factor+cloud);
}
vec3 render2(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 100);
	if(!cameraCast.hit) return skyColor(rd); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;

	vec3 lightDir = normalize(light1dir);
	diffuse = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 20.0, 100))*diffuse*0.4 + vec3(0.1);

	vec3 lightDir2 = normalize(light2dir);
	vec3 diffuse2 = vec3(1.0, 1.0, 0.1)*max(vec3(0.0), dot(-lightDir2, cameraCast.normal))*0.8;
	cool += vec3(worldShadow(cameraCast.position, -lightDir2, 200.0, 100))*diffuse2;

	// cool += render3(cameraCast.position + cameraCast.normal*0.01, reflect(rd, cameraCast.normal))*0.8*vec3(0.7, 0.9, 1.0);

	return cool;
}

vec3 render(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 200);
	if(!cameraCast.hit) return skyColor(rd); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;

	vec3 lightDir = normalize(light1dir);
	diffuse = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 60.0, 100))*diffuse*0.4 + vec3(0.);

	vec3 lightDir2 = normalize(light2dir);
	vec3 diffuse2 = vec3(1.0, 1.0, 0.1)*max(vec3(0.0), dot(-lightDir2, cameraCast.normal))*0.8;
	diffuse2*( (sin(cameraCast.position.x)+sin(cameraCast.position.x)+sin(cameraCast.position.x))*0.2 + 0.8);
	cool += vec3(worldShadow(cameraCast.position, -lightDir2, 200.0, 200))*diffuse2;

	if(cameraCast.material.shininess>0.5){
		cameraCast.normal=normalize(cameraCast.normal+sin((cameraCast.position+vec3(TIME))*5)*0.01+sin((cameraCast.position+vec3(TIME)))*0.04);
		cool += render2(cameraCast.position + cameraCast.normal*0.01, reflect(rd, cameraCast.normal))*0.8*vec3(0.7, 0.9, 1.0)*(1.1-dot(cameraCast.normal, normalize(ro - cameraCast.position)))*cameraCast.material.shininess;
	}
	return cool;
}

Camera getCam()
{
	Camera cam;
	vec3 lookAt = vec3(0, -12, 0);

	cam.position = fpar[1].xyz;

	// figure out camera space from position and lookAt
	cam.up = vec3(0, 1, 0);
	cam.forwards = normalize(fpar[2].xyz);
	cam.left = normalize(cross(cam.forwards, cam.up));
	cam.up = normalize(cross(cam.left, cam.forwards));

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

	if(gl_FragCoord.x<20&&gl_FragCoord.y<20){
		color = vec4(cam1.up, 1.0);
	}
	else{
		color = vec4(render(cam1.position, cam1.rayDir), 1.0);
	}
}
