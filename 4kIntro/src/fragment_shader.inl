/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef FRAGMENT_SHADER_INL_
# define FRAGMENT_SHADER_INL_

const char *fragment_shader_glsl_pr =
 "#version 430\n"
 "layout(location=0)uniform vec4 fpar[4];"
 "layout(location=0)out vec4 color;"
 "in vec2 p;"
 "vec3 greenish()"
 "{"
   "return vec3(.2,.9,.35);"
 "}"
 "void main()"
 "{"
   "vec2 v=gl_FragCoord.xy/vec2(1280,720),n=vec2(v*20.);"
   "color=vec4(greenish()*v.x,1.);"
 "}";

#endif // FRAGMENT_SHADER_INL_
