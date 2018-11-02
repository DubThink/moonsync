/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef FRAGMENT_SHADER_INL_
# define FRAGMENT_SHADER_INL_

const char *fragment_shader_glsl =
 "#version 430\n"
 "layout(location=0)uniform vec4 fpar[4];"
 "layout(location=0)out vec4 color;"
 "in vec2 p;"
 "void main()"
 "{"
   "float v;"
   "v=fpar[0].x;"
   "vec2 c=vec2(1920.,1080.),l=gl_FragCoord.xy-c/2.;"
   "l=vec2(length(l)/c.y-.3,atan(l.y,l.x));"
   "vec4 y=.07*cos(1.5*vec4(0,1,2,3)+v+l.y+sin(l.y)*cos(v)),x=y.yzwx,n=max(l.x-y,x-l.x);"
   "color=dot(clamp(n*c.y,0.,1.),72.*(y-x))*(y-.1)+n;"
 "}";

#endif // FRAGMENT_SHADER_INL_
