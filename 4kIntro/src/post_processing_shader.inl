/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef POST_PROCESSING_SHADER_INL_
# define POST_PROCESSING_SHADER_INL_

const char *post_processing_shader_glsl_pr =
 "#version 430\n"
 "layout(location=0)uniform vec4 fpar[4];"
 "layout(location=0)out vec4 color;"
 "uniform sampler2D inputTexture;"
 "in vec2 p;"
 "vec3 greenish()"
 "{"
   "return vec3(.2,.9,.35);"
 "}\n"
 "#define time fpar[0].x\n"
 "void main()"
 "{"
   "vec2 v=.5*p+.5,l=v-vec2(.5,.5);"
   "vec3 f=greenish();"
   "float t=.2+.1*cos(atan(l.y,l.x)*10.+20.*l.x+sin(time));"
   "f*=smoothstep(t,t+.01,length(l));"
   "color=vec4(f,1.);"
 "}";

#endif // POST_PROCESSING_SHADER_INL_
