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
#include "debug_camera.h"

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

static float fparams[4 * 4];

HWND hWnd;
int  intro_init(HWND h){
	hWnd = h;

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

	fparams[1] = XRES;
	fparams[2] = YRES;

	return 1;
}

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
DebugCamera cam;
long last_time=-1;
float frameTime = 0;
bool captureMouse = false;
bool lastCaptureKey = false;

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
	if (last_time > 0)
		frameTime = time - last_time;
	last_time = time;

	captureMouse = GetAsyncKeyState(VK_TAB);

	
	// -------------- CAMERA CONTROL
	if (hWnd == GetForegroundWindow()) {
		cam.speed = (GetAsyncKeyState(VK_SHIFT) ? .8 : 0.15);
		cam.frameTime=frameTime/20.0;
		if (GetAsyncKeyState('W'))cam.moveForward(1);
		if (GetAsyncKeyState('S'))cam.moveForward(-1);
		if (GetAsyncKeyState('A'))cam.moveRight(-1);
		if (GetAsyncKeyState('D'))cam.moveRight(1);
		if (GetAsyncKeyState(VK_SPACE))cam.moveUp(.75);
		if (GetAsyncKeyState(VK_CONTROL))cam.moveUp(-.75); 
		if (GetAsyncKeyState(VK_LEFT))cam.lookRight(-2);
		if (GetAsyncKeyState(VK_RIGHT))cam.lookRight(2);
		if (GetAsyncKeyState(VK_DOWN))cam.lookUp(-1);
		if (GetAsyncKeyState(VK_UP))cam.lookUp(1);

		POINT screenMouse;
		GetCursorPos(&screenMouse);
		POINT windowMouse(screenMouse);
		POINT delta;


		if (ScreenToClient(hWnd, &windowMouse))
		{
			//p.x and p.y are now relative to hwnd's client area
			delta.x = windowMouse.x - XRES / 2;
			delta.y = windowMouse.y - YRES / 2;
			SetCursorPos(screenMouse.x - delta.x, screenMouse.y - delta.y);
			cam.lookRight(delta.x*MOUSE_SENSE);
			cam.lookUp(-delta.y*MOUSE_SENSE);
			ShowCursor(FALSE);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindProgramPipeline(renderingPipeline);

	// Set fparams to give input to shaders
	/*
	fparams[0][0] = time in seconds
	fparams[0][1] = XRES
	fparams[0][2] = YRES
	fparams[1].xyz = camera pos
	fparams[2].xyz = look dir
	*/
	fparams[0] = time / 1000.0f;
	Vec3 camPos = cam.getPosition();
	Vec3 camLook = cam.getLookDirection();
	fparams[4] = (float)camPos.x;
	fparams[5] = (float)camPos.y;
	fparams[6] = (float)camPos.z;
	fparams[8] = (float)camLook.x;
	fparams[9] = (float)camLook.y;
	fparams[10] = (float)camLook.z;
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