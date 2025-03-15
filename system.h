//
//  Video Header
//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <Carbon/Carbon.h>
// #import <Cocoa/Cocoa.h>
// #import <QuartzCore/CAMetalLayer.h>
// #import <Metal/Metal.h>
#include <core/core.h>
#include <core/sys.h>
#include <core/sysvideo.h>
#include <core/math.h>


// typedef enum {
// 	KEY_A=kVK_ANSI_A, KEY_B=kVK_ANSI_B, KEY_C=kVK_ANSI_C, KEY_D=kVK_ANSI_D, KEY_E=kVK_ANSI_E, KEY_F=kVK_ANSI_F,
// 	KEY_G=kVK_ANSI_G, KEY_H=kVK_ANSI_H, KEY_I=kVK_ANSI_I, KEY_J=kVK_ANSI_J, KEY_K=kVK_ANSI_K, KEY_L=kVK_ANSI_L,
// 	KEY_M=kVK_ANSI_M, KEY_N=kVK_ANSI_N, KEY_O=kVK_ANSI_O, KEY_P=kVK_ANSI_P, KEY_Q=kVK_ANSI_Q, KEY_R=kVK_ANSI_R,
// 	KEY_S=kVK_ANSI_S, KEY_T=kVK_ANSI_T, KEY_U=kVK_ANSI_U, KEY_V=kVK_ANSI_V, KEY_W=kVK_ANSI_W, KEY_X=kVK_ANSI_X,
// 	KEY_Y=kVK_ANSI_Y, KEY_Z=kVK_ANSI_Z,

// 	KEY_1=kVK_ANSI_1, KEY_2=kVK_ANSI_2, KEY_3=kVK_ANSI_3, KEY_4=kVK_ANSI_4, KEY_5=kVK_ANSI_5,
// 	KEY_6=kVK_ANSI_6, KEY_7=kVK_ANSI_7, KEY_8=kVK_ANSI_8, KEY_9=kVK_ANSI_9, KEY_0=kVK_ANSI_0,
	
// 	KEY_F1=kVK_F1, KEY_F2=kVK_F2, KEY_F3=kVK_F3, KEY_F4=kVK_F4, KEY_F5=kVK_F5, KEY_F6=kVK_F6,
// 	KEY_F7=kVK_F7, KEY_F8=kVK_F8, KEY_F9=kVK_F9, KEY_F10=kVK_F10, KEY_F11=kVK_F11, KEY_F12=kVK_F12,
	
// 	KEY_LEFT=kVK_LeftArrow, KEY_UP=kVK_UpArrow, KEY_RIGHT=kVK_RightArrow, KEY_DOWN=kVK_DownArrow,
	
// 	KEY_RETURN=kVK_Return, KEY_BACK=kVK_Delete, KEY_SPACE=kVK_Space, KEY_ESC=kVK_Escape, KEY_TAB=kVK_Tab,
// 	KEY_SHIFT=kVK_Shift, KEY_RSHIFT=kVK_RightShift, KEY_CONTROL=kVK_Control, KEY_RCONTROL=kVK_RightControl,
// 	KEY_MENU=kVK_Option, KEY_RMENU=kVK_RightOption, KEY_COMMAND=kVK_Command, KEY_RCOMMAND=kVK_RightCommand,
// } sys_key_code_t;

typedef struct {
	// /*NSApplication**/ void* app;
	// /*NSWindow**/ void* window;

	// /*id<MTLDevice>*/ void* device;
	// /*CAMetalLayer**/ void* metalLayer;
	// /*id<MTLCommandQueue>*/ void* commandQueue;
	// /*id<MTLRenderPipelineState>*/ void* pipeline;

	u8 objc_state[128];

	int2_t screenSize;
	int2_t framebufferSize;

	// /*id<MTLTexture>*/ void* framebufferTexture;
	u32* framebuffer;
	u32* scaledFramebuffer;

	vec2_t worldSpaceMin;
	vec2_t worldSpaceMax;
	vec2_t worldSpace;

	// button_t keyboard[256];
	// mouse_t mouse;
} video_t;

// typedef struct {
// 	union {
// 		i16 channels[2];
// 		struct {
// 			i16 left;
// 			i16 right;
// 		};
// 	};
// } sys_audio_sample_t;
// typedef struct {
// 	int channels;
// 	int samplesPerSecond;
// 	int bytesPerSample;
// 	size_t sampleCount;
// 	sys_audio_sample_t data[];
// } sys_wave_t;

// typedef struct {
// 	sys_wave_t* wave;
// 	float cursor;
// 	float volume;
// } sys_sound_t;

// typedef void (*audio_mixer_proc)(void* outputStream, int sampleCount, void* userdata);

#define SYS_FUNC

SYS_FUNC void Sys_InitMetal(sys_window_t* win, video_t* video);
SYS_FUNC void Sys_OutputFrameAndSync(sys_window_t* win, video_t* video);
// SYS_FUNC void Sys_InitWindow();
// SYS_FUNC void Sys_InitMetal();
// SYS_FUNC void Sys_PollEvents(window_t* win);

// SYS_FUNC void Sys_QueueSound(sys_wave_t* wave, float volume);
// SYS_FUNC void Sys_InitAudio(audio_mixer_proc mixerProc);
// SYS_FUNC sys_wave_t* Sys_LoadWave(allocator_t* allocator, file_data_t* fileData);
char* Sys_GetResourcePath(allocator_t* allocator, char* filename);

#endif
