#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "ext.h"
#include "vertex_shader.inl"
#include "fragment_shader.inl"
#include "post_processing_shader.inl"
#include "fp.h"
#include "debug_help.h"

//remove
#include <direct.h>
//#include <string.h>

GLuint fragmentShader;
static GLuint framebuffer;
static GLuint texture;
static GLuint depthTexture;
static GLuint renderingPipeline;
static GLuint postProcessingPipeline;
GLuint postProcessingShader;

#ifdef DEBUG
HWND hWnd;
int  intro_init(HWND h){
	hWnd = h;
#else
int  intro_init(void){
#endif

	if (!EXT_Init())
		return 0;

	GLuint vertexShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertex_shader_glsl);
	fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragment_shader_glsl);
	postProcessingShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &post_processing_shader_glsl);

	glGenProgramPipelines(1, &renderingPipeline);
	glBindProgramPipeline(renderingPipeline);
	glUseProgramStages(renderingPipeline, GL_VERTEX_SHADER_BIT, vertexShader);
	glUseProgramStages(renderingPipeline, GL_FRAGMENT_SHADER_BIT, fragmentShader);
	//glUseProgramStages(renderingPipeline, GL_FRAGMENT_SHADER_BIT, 0);
	//glUseProgramStages(renderingPipeline, GL_FRAGMENT_SHADER_BIT, fragmentShader);

	glGenProgramPipelines(1, &postProcessingPipeline);
	glBindProgramPipeline(postProcessingPipeline);
	glUseProgramStages(postProcessingPipeline, GL_VERTEX_SHADER_BIT, vertexShader);
	glUseProgramStages(postProcessingPipeline, GL_FRAGMENT_SHADER_BIT, postProcessingShader);

#ifdef DEBUG
	int result;
	char info[1536];
	glGetProgramiv(vertexShader, GL_LINK_STATUS, &result);
	glGetProgramInfoLog(vertexShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
	glGetProgramiv(fragmentShader, GL_LINK_STATUS, &result);
	glGetProgramInfoLog(fragmentShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
	glGetProgramiv(postProcessingShader, GL_LINK_STATUS, &result);
	glGetProgramInfoLog(postProcessingShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
#endif

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, XRES, YRES);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, XRES, YRES);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	static const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	return 1;
}

static float fparams[4 * 4];

#ifdef DEBUG
long last_load = 0; // last shader load
void reloadShaders() {
	char cwd[300];
	_getcwd(cwd, 300);
	//MessageBox(0, cwd, ":)", MB_OK | MB_ICONINFORMATION);
	char* shader = loadShader("src/fragment_shader.glsl");
	//MessageBox(0, shader, ":)", MB_OK | MB_ICONINFORMATION);

	int result;
	char info[1536];

	if (shader != nullptr) {
		fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &shader);
		glGetProgramiv(fragmentShader, GL_LINK_STATUS, &result);
		glGetProgramInfoLog(fragmentShader, 1024, NULL, (char *)info);
		if (!result) {
			MessageBox(0, info, "Frag Error!", MB_OK | MB_ICONEXCLAMATION);
		}
		glUseProgramStages(renderingPipeline, GL_FRAGMENT_SHADER_BIT, fragmentShader);
	}
	shader = loadShader("src/post_processing_shader.glsl");
	if (shader != nullptr) {
		glGetProgramiv(postProcessingShader, GL_LINK_STATUS, &result);
		glGetProgramInfoLog(postProcessingShader, 1024, NULL, (char *)info);
		if (!result) {
			MessageBox(0, info, "PostProcessing Error!", MB_OK | MB_ICONEXCLAMATION);
		}
		postProcessingShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &shader);
		glUseProgramStages(postProcessingPipeline, GL_FRAGMENT_SHADER_BIT, postProcessingShader);
	}
}
#endif

void intro_do(long time)
{
#ifdef DEBUG
	// Listen to CTRL+S.
	if (hWnd == GetForegroundWindow()&&GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S'))
	{
		// Wait for a while to let the file system finish the file write.
		if (time - last_load > 200) {
			Sleep(100);
			reloadShaders();
		}
		last_load = time;
		//MessageBox(0, "Shaders Loaded", "Shaders Loaded", MB_OK | MB_ICONINFORMATION);
	}
#endif
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindProgramPipeline(renderingPipeline);
	// Set fparams to give input to shaders
	/*
	fparams[0][0] = time in seconds
	*/
	fparams[0] = time / 1000.0f;
	// Render
	glProgramUniform4fv(fragmentShader, 0, 4, fparams);
	glRects(-1, -1, 1, 1); // Deprecated. Still seems to work though.

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindProgramPipeline(postProcessingPipeline);
	glBindTexture(GL_TEXTURE_2D, texture);
	glRects(-1, -1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
}