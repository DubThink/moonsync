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

struct Light{
	vec3 position;
	vec3 color;
	float radius;
	float PLACEHOLDER;
};

#define NR_LIGHTS 16
layout (std140, binding = 1) uniform LightBlock {
    Light lights[NR_LIGHTS];
};

vec2 resolution = vec2(fpar[0].y, fpar[0].z); // TODO: get this as a uniform

#define TIME fpar[0].x

//	Classic Perlin 3D Noise
//	by Stefan Gustavson
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvsqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec3 P){
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 / 7.0;
  vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 / 7.0;
  vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvsqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvsqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
  return 2.2 * n_xyz;
}

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


Light sun = Light(vec3(-0.3, -0.8, 0.2),vec3(1.0,1.0,0.9));

vec3 light1dir=vec3(0.6, -1.0, -0.2);
vec3 light2dir=vec3(-0.2, -0.8, 0.2);



// ----------------------------------------------------- //
// SDF functions
// ----------------------------------------------------- //

const vec3 I = vec3(1,0,0);
const vec3 J = vec3(0,1,0);

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);

    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}

mat3 pointatY(in vec3 dir){
	float _cos = dot(normalize(-dir.xz),I.xy);
	float _sin = dot(normalize(-vec2(dir.z,-dir.x)),I.xy);
	mat3 roty=mat3(_cos,0,_sin,0,1,0,-_sin,0,_cos);
	return roty;
}
mat3 pointatX(in vec3 dir){
	dir=normalize(dir);
	float theta = asin(dir.y);
	mat3 rotx =  mat3(
		cos(theta),-sin(theta),0,
		sin(theta),cos(theta),0,
		0,0,1);
	return rotx;
}

// copy-pasted from Iv
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
vec3 waterpos[6];
float worldSDF2(in vec3 v)
{
	float sdf_agg = 0.0;

	sdf_agg = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));

	for(int i=0;i<6;i++){
		sdf_agg=opSmoothUnion(sdf_agg,sphereSDF(v,waterpos[i]*vec3(1,3,1),1),.5);
	}
	return sdf_agg;
}

float sdEnemy(in vec3 v,in vec3 p){
	vec3 toCam=normalize(p-fpar[1].xyz);
	v-=p;

	v=v*pointatY(toCam);
	v=v*pointatX(toCam);
	float playerModel = sphereSDF(v,vec3(0),1);//sdVerticalCapsule(v,2,1);
	playerModel = opSmoothSubtraction(
		min(sphereSDF(vec3(v.x,v.y,abs(v.z)),vec3(.6,.3,.4),.3),
		sphereSDF(vec3(v.x,v.y,v.z),vec3(.6,-.3,0),.4)),
		playerModel,.1);
	return min(sphereSDF(vec3(v.x,v.y,abs(v.z)),vec3(.6,.3,.4),.1),playerModel);
}

float worldSDF1(in vec3 v)
{
	float sdf_agg = 0.0;

	float plane = sdPlane(v, vec4(0, 1.0, 0.0, 1.0));
	vec3 q,r;

	// enemy
	float playerModel = sdEnemy(v,vec3(0,16,10));

	float box1 = sdBox(v, vec3(36.0, 22.0, 16.0));
	float box2 = sdBox(v, vec3(34.5, 35.0, 14.5));
	// -- Arches - lower floor
	float archesLowerFloor = min(sdCappedCylinder(vec3(v.y-6,v.x,mod(v.z+5,10)-5),vec2(4.5,36)),sdCappedCylinder(vec3(mod(v.x+5,10)-5,v.z,v.y-6),vec2(4.5,15)));
	archesLowerFloor = opSubtraction(archesLowerFloor, sdBox(v-vec3(0,8.5,0),vec3(36.0,2.5, 15.0)));
	float mainPillars = sdCappedCylinder(vec3(mod(v.x,10)-5,v.y,mod(v.z,10)-5),vec2(.4,22));
	archesLowerFloor = opSmoothUnion(archesLowerFloor,mainPillars,.5);

	q=v-vec3(0,11,0);
	float archesUpperFloor = min(sdCappedCylinder(vec3(q.y-6,q.x,mod(q.z+5,10)-5),vec2(4.5,36)),sdCappedCylinder(vec3(mod(q.x+5,10)-5,q.z,q.y-6),vec2(4.5,15)));
	archesUpperFloor = opSubtraction(archesUpperFloor, sdBox(q-vec3(0,10.5,0),vec3(36.0,3.5, 15.0)));

	float arches = opSmoothUnion(archesUpperFloor,archesLowerFloor,.5);
	arches=min(arches,sdBox(v+vec3(0,0.75,0),vec3(36.0,.5, 15.0)));
	arches = min(arches,min( sdBox(vec3(v.x-0,v.y-12,abs(v.z)-5), vec3(25.0, 1.,.25)),sdBox(vec3(abs(v.x)-25,v.y-12,v.z), vec3(.25, 1.,5))));
	sdf_agg = min(opSubtraction(sdBox(v,vec3(24,35,4.5)),min(arches,opSubtraction(box2,box1))), playerModel);
	// float blob = sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25 + sin(v.x) * sin(v.y) * sin(v.z)*0.1; // blobs on this sphere

	// vec3 v = v.xyz;

	// v.xz = mod(v.xz, vec2(20.0, 20.0)) - vec2(10.0, 10.0);

	// float sphere1 = sphereSDF(v, vec3(-0.0,10 - sin(v.x/10.0)*3.0 - sin(v.z/10.0),0), 4.0);
	//
	// float sphere2 = sphereSDF(v, vec3(-5.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 0.0), 2.0);
	// float sphere3 = sphereSDF(v, vec3(0.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 5.0), 2.0);
	// float sphere4 = sphereSDF(v, vec3(0.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, -5.0), 2.0);
	// float sphere5 = sphereSDF(v, vec3(5.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 0.0), 2.0);
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

// Tetrahedron technivue from http://ivuilezles.org/www/articles/normalsSDF/normalsSDF.htm
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
	// default material
	marched.material.diffuseColor=vec3(1);
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
		marched.material.shininess=0.8;
	}else {
		marched.material.shininess=0.2;
		marched.material.diffuseColor=vec3(0.1);

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
	float cloud=max(0,cnoise(v*vec3(2,6,2)+vec3(TIME*0.1)) + cnoise(5*v*vec3(2,6,2)+vec3(TIME*0.1))*0.2);
	return mix( vec3(.4,.5,1.),vec3(.5,.5,1.),factor)+cloud*factor;
}
vec3 render2(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 100);
	if(!cameraCast.hit) return skyColor(rd); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;
	vec3 cool;

	vec3 sunAlbedo = sun.color*max(vec3(0.0), dot(-normalize(sun.position), cameraCast.normal));
	sunAlbedo*( (sin(cameraCast.position.x)+sin(cameraCast.position.x)+sin(cameraCast.position.x))*0.2 + 0.8);
	cool += vec3(worldShadow(cameraCast.position, -sun.position, 200.0, 200))*sunAlbedo;

	return cool;
}

vec3 render(in vec3 ro, in vec3 rd)
{
	raymarchResult cameraCast = worldMarch(ro, rd, 200);
	if(!cameraCast.hit) return skyColor(rd); // sky color
	vec3 diffuse = cameraCast.material.diffuseColor;
	vec3 cool=diffuse*0.8*(
		max(0,dot(cameraCast.normal,vec3(0,0.7,0.7)))+
		max(0,dot(cameraCast.normal,vec3(0,-0.7,0.7)))+
		max(0,dot(cameraCast.normal,vec3(0.4,0.3,-0.7)))
		)*vec3(.7,.5,1);
	// vec3 lightDir = normalize(light1dir);
	// lightam = max(vec3(0.0), dot(-lightDir, cameraCast.normal))*0.5*vec3(0.4, 0.8, 1.0);
	// vec3 cool = vec3(worldShadow(cameraCast.position, -lightDir, 60.0, 100))*diffuse*0.4 + vec3(0.);

	vec3 sunAlbedo = sun.color*max(vec3(0.0), dot(-normalize(sun.position), cameraCast.normal));
	sunAlbedo*( (sin(cameraCast.position.x)+sin(cameraCast.position.x)+sin(cameraCast.position.x))*0.2 + 0.8);
	cool += vec3(worldShadow(cameraCast.position, -sun.position, 200.0, 200))*sunAlbedo;

	if(cameraCast.material.shininess>0.5){
		cameraCast.normal=normalize(cameraCast.normal+vec3(cnoise(vec3(cameraCast.position.xz*0.5,TIME*0.4)),0,0)*0.08+vec3(cnoise(vec3(cameraCast.position.xz*2,TIME)),0,0)*0.03);
		vec3 reflection = render2(cameraCast.position + cameraCast.normal*0.01, reflect(rd, cameraCast.normal))*0.8*vec3(0.7, 0.9, 1.0)*(1.1-dot(cameraCast.normal, normalize(ro - cameraCast.position)))*cameraCast.material.shininess;
		cool = mix(cool,reflection,cameraCast.material.shininess);
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
	for(int i=0;i<6;i++){
		waterpos[i]=vec3(cnoise(vec3(i,TIME*2,0)),cnoise(vec3(i,TIME*2,10)),cnoise(vec3(i,TIME*2,20)));
	}
	Camera cam1 = getCam();

	color = vec4(render(cam1.position, cam1.rayDir), 1.0);
}
