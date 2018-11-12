/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef POST_PROCESSING_SHADER_INL_
# define POST_PROCESSING_SHADER_INL_

const char *post_processing_shader_glsl_pr =
 "#version 430\n"
 "#line 1 0\n"
 "layout(location=0)uniform vec4 fpar[4];"
 "layout(location=0)out vec4 color;"
 "uniform sampler2D inputTexture;"
 "in vec2 p;\n"
 "#define time fpar[0].x;\n"
 "void main()"
 "{"
   "vec2 v=.5*p+.5;"
   "color=texture(inputTexture,v);"
 "}";

#endif // POST_PROCESSING_SHADER_INL_
