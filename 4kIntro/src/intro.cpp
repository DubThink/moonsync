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
#include "vec3.h"
#include "ballPhysics.h"
#include "worldsdf.h"
#include "v2mplayer.h"
#include "libv2.h"
//remove
//#include <direct.h>
//#include <string.h>

GLuint fragmentShader;
static GLuint framebuffer;
static GLuint texture;
static GLuint depthTexture;
static GLuint renderingPipeline;
static GLuint postProcessingPipeline;
GLuint postProcessingShader;

static Vec4 fparams[4];

#define NR_LIGHTS 8
struct Light {
	Vec4 pos;
	Vec4 color;
};
GLuint mylightbuffer;
Light lights[NR_LIGHTS];

GLuint myballbuffer;
Ball myballs[NR_BALLS];
PhysBall* playerBall = allMyBalls;

HWND hWnd;


// -------- sound block  ---------
static V2MPlayer player;
static V2MPlayer playerZZZ;
extern "C" const sU8 theTune[];
extern "C" const sU8 theZZZ[];

void  InitSound()
{
	player.Init();
	player.Open(theTune);

	dsInit(player.RenderProxy, &player, GetForegroundWindow());

	playerZZZ.Init();
	playerZZZ.Open(theZZZ);

	//dsInit(playerZZZ.RenderProxy, &playerZZZ, GetForegroundWindow());

	player.Play();
	playerZZZ.Play();
}
void CloseSound() {
	dsClose();
	player.Close();
}
// --------  ---------

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
	glGetProgramInfoLog(fragmentShader, 1024, NULL, (char *)info);
	if (!result) {
		MessageBox(0, info, "Frag Error!", MB_OK | MB_ICONEXCLAMATION);
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

	fparams[0].y = XRES;
	fparams[0].z = YRES;

	//glUniformBlockBinding(fragmentShader, 0, 0);

	// allocate memory for light buffer
	glGenBuffers(1, &mylightbuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, mylightbuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * NR_LIGHTS, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// bind light buffer to location 0
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, mylightbuffer,0, sizeof(Light) * NR_LIGHTS);

	// allocate memory for balls buffer
	glGenBuffers(1, &myballbuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, myballbuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Ball) * NR_BALLS, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 1);

	// bind balls buffer to location 1
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, myballbuffer, 0, sizeof(Ball) * NR_BALLS);

	for (int i = 0; i < NR_BALLS; i++) {
		allMyBalls[i].position = vec3(i*2 - 10, 10, 0);
		allMyBalls[i].radius = 0.5 + 0.1*i;
	}
	playerBall->radius = 1;
	playerBall->restitution = 0.0;
	playerBall->friction = 0.5;
	playerBall->playerPhysics = true;

	lights[0].pos = Vec4{ 24.f, 14.f, 0.f,0.f };
	lights[0].color = Vec4{ 1.0,0.95f,0.5f,0.f };
	lights[1].pos = Vec4{-24.f, 14.f, 0.f,0.f };
	lights[1].color = Vec4{ 0.8f,0.95f,0.95f,0.f };

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
bool freeCam = true;

// action bools
bool tabWasLastPressed = false;
bool lbuttonWasLastPressed = false;
bool rbuttonWasLastPressed = false;

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

	// action keys
	if (GetAsyncKeyState(VK_TAB) && !tabWasLastPressed) {
		// tab action key
		freeCam = !freeCam;
	}
	tabWasLastPressed = GetAsyncKeyState(VK_TAB);
	
	updatePhysics(frameTime / 1000.0);
	copyBalls(myballs);
	if (playerBall->onground)playerBall->velocity.y = 0;

	// -------------- CAMERA CONTROL
	if (hWnd == GetForegroundWindow()) {
		cam.speed = (GetAsyncKeyState(VK_SHIFT) ? .8 : 0.15);
		cam.frameTime=frameTime/20;
		float physSpeed = playerBall->onground?.3:.01;
		if (freeCam) {
			if (GetAsyncKeyState('W')) cam.moveForward(1);
			if (GetAsyncKeyState('S')) cam.moveForward(-1);
			if (GetAsyncKeyState('A')) cam.moveRight(-1);
			if (GetAsyncKeyState('D')) cam.moveRight(1);
			if (GetAsyncKeyState(VK_SPACE))cam.moveUp(.75);
			if (GetAsyncKeyState(VK_CONTROL))cam.moveUp(-.75);

		} else {
			if ((GetAsyncKeyState('W') || GetAsyncKeyState('S') || GetAsyncKeyState('A') || GetAsyncKeyState('D'))&&playerBall->onground) {
				playerBall->velocity.x = 0;
				playerBall->velocity.z = 0;
			}
			if (GetAsyncKeyState('W')) playerBall->velocity += normalize(cam.getLookDirection().x0z())*physSpeed*frameTime;
			if (GetAsyncKeyState('S')) playerBall->velocity += normalize(cam.getLookDirection().x0z())*physSpeed*-frameTime;
			if (GetAsyncKeyState('A')) playerBall->velocity += normalize(cross(cam.getLookDirection(), vec3{ 0,1,0 }))*physSpeed*-frameTime;
			if (GetAsyncKeyState('D')) playerBall->velocity += normalize(cross(cam.getLookDirection(), vec3{ 0,1,0 }))*physSpeed*frameTime;
			if (GetAsyncKeyState(VK_SPACE) && playerBall->onground && playerBall->velocity.y<8.0f)playerBall->velocity += vec3{ 0,12,0 };

		}
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
	fparams[0].x = time / 1000.0f;
	vec3 camPos = freeCam ? cam.getPosition() : playerBall->position + vec3(0, 2.5, 0);;
	vec3 camLook = cam.getLookDirection();
	fparams[1].x = (float)camPos.x;
	fparams[1].y = (float)camPos.y;
	fparams[1].z = (float)camPos.z;
	fparams[2].x = (float)camLook.x;
	fparams[2].y = (float)camLook.y;
	fparams[2].z = (float)camLook.z;

	if (GetAsyncKeyState(VK_LBUTTON) && !lbuttonWasLastPressed) {
		// tab action key
		allMyBalls[3].position = vec3(fparams[1]);
		allMyBalls[3].velocity = cam.getLookDirection() * 20;
		dsLock();
		player.Play(150);
		dsUnlock();
		
	} lbuttonWasLastPressed = GetAsyncKeyState(VK_LBUTTON);

	if (GetAsyncKeyState(VK_RBUTTON)) {// && !rbuttonWasLastPressed
		RaymarchResult result = worldMarch(camPos,camLook, 100, 0.5f,12);
		if(result.hit)result.position += result.normal * -1;
		lights[2].pos = Vec4{result.position.x,result.position.y,result.position.z,1.0};
	}
	else {
		lights[2].pos = Vec4{ 0.0, 0.0, 0.0, 0.0 };
	}
	rbuttonWasLastPressed = GetAsyncKeyState(VK_RBUTTON);
	
	glBindBuffer(GL_UNIFORM_BUFFER, mylightbuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light)*NR_LIGHTS, lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	//RaymarchResult result = worldMarch(vec3(0, 15, 0), vec3(sin(time / 5000.f), 0, cos(time / 5000.f)), 800,0.5f);
	//myballs[0].pos = Vec4{ result.position.x,result.position.y,result.position.z,0.5f};



	glBindBuffer(GL_UNIFORM_BUFFER, myballbuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Ball)*NR_BALLS, myballs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Render
	glProgramUniform4fv(fragmentShader, 0, 4, (float*)fparams);
	glRects(-1, -1, 1, 1); // Deprecated. Still seems to work though.

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindProgramPipeline(postProcessingPipeline);
	glBindTexture(GL_TEXTURE_2D, texture);
	glProgramUniform4fv(postProcessingShader, 0, 4, (float*)fparams);
	glRects(-1, -1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
}