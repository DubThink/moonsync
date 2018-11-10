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
	fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragment_shader_glsl_pr);
	postProcessingShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &post_processing_shader_glsl_pr);

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
		MessageBox(0, info, "Error Vert!", MB_OK | MB_ICONEXCLAMATION);
	}
	glGetProgramiv(fragmentShader, GL_LINK_STATUS, &result);
	glGetShaderInfoLog(fragmentShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Frag Error! (1)", MB_OK | MB_ICONEXCLAMATION);
	}
	glGetProgramInfoLog(fragmentShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Frag Error! (2)", MB_OK | MB_ICONEXCLAMATION);
	}
	glGetProgramiv(postProcessingShader, GL_LINK_STATUS, &result);
	glGetProgramInfoLog(postProcessingShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Error Post!", MB_OK | MB_ICONEXCLAMATION);
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
// from https://stackoverflow.com/questions/12554237/hiding-command-prompt-called-by-system
/* Silently runs system commands in the background. */
int system_hidden(const char *cmdArgs)
{
	PROCESS_INFORMATION pinfo;
	STARTUPINFO sinfo;

	/*
	 * Allocate and hide console window
	 */
	AllocConsole();
	ShowWindow(GetConsoleWindow(), 0);

	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	CreateProcess(NULL, (char*)cmdArgs,
		NULL, NULL, false,
		0,
		NULL, NULL, &sinfo, &pinfo);
	DWORD ret;
	while (1)
	{
		HANDLE array[1];
		array[0] = pinfo.hProcess;
		ret = MsgWaitForMultipleObjects(1, array, false, INFINITE,
			QS_ALLPOSTMESSAGE);
		if ((ret == WAIT_FAILED) || (ret == WAIT_OBJECT_0))
			break;
		/*
		 * Don't block message loop
		 */
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DWORD pret;
	GetExitCodeProcess(pinfo.hProcess, &pret);
	//    FreeConsole ();
	return pret;
}
void reloadFragmentShader() {
	int result;
	char info[1536];
	system_hidden("cmd /c python src\\preprocessor.py src\\fragment_shader.glsl");
	char* shader = loadShader("src/fragment_shader.glsl.pr");
	//MessageBox(0, shader, "Frag Shader", MB_OK | MB_ICONEXCLAMATION);
	if (shader != nullptr) {
		fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &shader);
		glGetProgramiv(fragmentShader, GL_LINK_STATUS, &result);
		glGetProgramInfoLog(fragmentShader, 1024, NULL, (char *)info);
		if (!result) {
			MessageBox(0, info, "Frag Error!", MB_OK | MB_ICONEXCLAMATION);
		}
		glUseProgramStages(renderingPipeline, GL_FRAGMENT_SHADER_BIT, fragmentShader);
	}
	else
		MessageBox(0, info, "Unable to load frag shader!", MB_OK | MB_ICONEXCLAMATION);
}
void reloadPostShader(){
	int result;
	char info[1536];
	system_hidden("cmd /c python src\\preprocessor.py src\\post_processing_shader.glsl");
	char* shader = loadShader("src/post_processing_shader.glsl.pr");
	//MessageBox(0, shader, "Post Shader", MB_OK | MB_ICONEXCLAMATION);
	if (shader != nullptr) {
		postProcessingShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &shader);
		glGetProgramiv(postProcessingShader, GL_LINK_STATUS, &result);
		glGetProgramInfoLog(postProcessingShader, 1024, NULL, (char *)info);
		if (!result) {
			MessageBox(0, info, "PostProcessing Error!", MB_OK | MB_ICONEXCLAMATION);
		}
		glUseProgramStages(postProcessingPipeline, GL_FRAGMENT_SHADER_BIT, postProcessingShader);
	} else 
		MessageBox(0, info, "Unable to load post processing shader!", MB_OK | MB_ICONEXCLAMATION);

}
long last_load = 0; // last shader load
#endif

void intro_do(long time)
{
#ifdef DEBUG
	// Listen to CTRL+S.
	if (/*hWnd == GetForegroundWindow()&& */GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S'))
	{
		// Wait for a while to let the file system finish the file write.
		if (time - last_load > 200) {
			Sleep(200);
			reloadFragmentShader();
			reloadPostShader();
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
	fparams[1] = 1920;
	fparams[2] = 1080;
	// Render
	glProgramUniform4fv(fragmentShader, 0, 4, fparams);
	glRects(-1, -1, 1, 1); // Deprecated. Still seems to work though.

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindProgramPipeline(postProcessingPipeline);
	glBindTexture(GL_TEXTURE_2D, texture);
	glProgramUniform4fv(postProcessingShader, 0, 4, fparams);
	glRects(-1, -1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
}