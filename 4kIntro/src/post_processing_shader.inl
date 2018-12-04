/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef POST_PROCESSING_SHADER_INL_
# define POST_PROCESSING_SHADER_INL_

const char *post_processing_shader_glsl_pr =
 "#version 440\n"
 "#line 1 0\n"
 "layout(location=0)uniform vec4 fpar[4];"
 "layout(location=0)out vec4 color;"
 "uniform sampler2D inputTexture;"
 "in vec2 p;\n"
 "#define time fpar[0].x;\n"
 "vec2 resolution=vec2(fpar[0].y,fpar[0].z);"
 "void main()"
 "{"
   "vec2 i=.5*p+.5;"
   "color=max(vec4(0),texture(inputTexture,i));"
   "vec4 c;"
   "for(int v=-20;v<=20;v++)"
     "c+=max(vec4(0),texture(inputTexture,i+vec2(v/fpar[0].y,0))-vec4(.7))*.2*(1-abs(v*.05));"
   "for(int v=-20;v<=20;v++)"
     "c+=max(vec4(0),texture(inputTexture,i+vec2(0,v/fpar[0].z))-vec4(.7))*.2*(1-abs(v*.05));"
   "if(abs(distance(gl_FragCoord.xy,resolution.xy/2)-6)<1)"
     "color=vec4(.3,1.,.15,1.);"
   "float v=fpar[0].w;"
   "if(i.x<v*.03&&mod(i.x,.03)>.015&&i.y>.95)"
     "color=vec4(.3,1.,.15,1.);"
 "}";

#endif // POST_PROCESSING_SHADER_INL_
