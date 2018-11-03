/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef POST_PROCESSING_SHADER_INL_
# define POST_PROCESSING_SHADER_INL_

const char *post_processing_shader_glsl =
 "#version 430\n"
 "layout(location=0)out vec4 color;"
 "uniform sampler2D inputTexture;"
 "in vec2 p;"
 "void main()"
 "{"
   "vec2 r=.5*p+.5;"
   "if(r.x<.5)"
     "color=vec4(r.x,0,r.y,0);"
   "else"
     " color=texture(inputTexture,r);"
 "}";

#endif // POST_PROCESSING_SHADER_INL_
