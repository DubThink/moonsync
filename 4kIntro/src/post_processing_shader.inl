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
   "vec2 r=.5*p+.5;"
   "color=max(vec4(0),texture(inputTexture,r));"
   "vec4 v;"
   "for(int i=-20;i<=20;i++)"
     "v+=max(vec4(0),texture(inputTexture,r+vec2(i/fpar[0].y,0))-vec4(.7))*.2*(1-abs(i*.05));"
   "for(int i=-20;i<=20;i++)"
     "v+=max(vec4(0),texture(inputTexture,r+vec2(0,i/fpar[0].z))-vec4(.7))*.2*(1-abs(i*.05));"
   "color+=v/3;"
   "if(abs(distance(gl_FragCoord.xy,resolution.xy/2)-6)<1)"
     "color=vec4(.3,1.,.15,1.);"
 "}";

#endif // POST_PROCESSING_SHADER_INL_
