/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef PHYSICS_SHADER_INL_
# define PHYSICS_SHADER_INL_

const char *physics_shader_glsl_pr =
"#version 430 compatibility\
	#extension GL_ARB_compute_shader : enable\
	#extension GL ARB shader storage buffer object : enable;\
\
layout(std140, binding = 4) buffer Pos\
{\
	vec4 Positions[]; // array of structures\
};\
\
layout(std140, binding = 5) buffer Vel\
{\
	vec4 Velocities[]; // array of structures\
};\
layout(std140, binding = 6) buffer Col\
{\
	vec4 Colors[]; // array of structures\
};layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;";

#endif // PHYSICS_SHADER_INL_
