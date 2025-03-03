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
#include <core/math.h>
#include <core/video.h>


#define KEY_A       kVK_ANSI_A
#define KEY_B       kVK_ANSI_B
#define KEY_C       kVK_ANSI_C
#define KEY_D       kVK_ANSI_D
#define KEY_E       kVK_ANSI_E
#define KEY_F       kVK_ANSI_F
#define KEY_G       kVK_ANSI_G
#define KEY_H       kVK_ANSI_H
#define KEY_I       kVK_ANSI_I
#define KEY_J       kVK_ANSI_J
#define KEY_K       kVK_ANSI_K
#define KEY_L       kVK_ANSI_L
#define KEY_M       kVK_ANSI_M
#define KEY_N       kVK_ANSI_N
#define KEY_O       kVK_ANSI_O
#define KEY_P       kVK_ANSI_P
#define KEY_Q       kVK_ANSI_Q
#define KEY_R       kVK_ANSI_R
#define KEY_S       kVK_ANSI_S
#define KEY_T       kVK_ANSI_T
#define KEY_U       kVK_ANSI_U
#define KEY_V       kVK_ANSI_V
#define KEY_W       kVK_ANSI_W
#define KEY_X       kVK_ANSI_X
#define KEY_Y       kVK_ANSI_Y
#define KEY_Z       kVK_ANSI_Z

#define KEY_1       kVK_ANSI_1
#define KEY_2       kVK_ANSI_2
#define KEY_3       kVK_ANSI_3
#define KEY_4       kVK_ANSI_4
#define KEY_5       kVK_ANSI_5
#define KEY_6       kVK_ANSI_6
#define KEY_7       kVK_ANSI_7
#define KEY_8       kVK_ANSI_8
#define KEY_9       kVK_ANSI_9
#define KEY_0       kVK_ANSI_0

#define KEY_F1      kVK_F1
#define KEY_F2      kVK_F2
#define KEY_F3      kVK_F3
#define KEY_F4      kVK_F4
#define KEY_F5      kVK_F5
#define KEY_F6      kVK_F6
#define KEY_F7      kVK_F7
#define KEY_F8      kVK_F8
#define KEY_F9      kVK_F9
#define KEY_F10     kVK_F10
#define KEY_F11     kVK_F11
#define KEY_F12     kVK_F12

#define KEY_LEFT    kVK_LeftArrow
#define KEY_UP      kVK_UpArrow
#define KEY_RIGHT   kVK_RightArrow
#define KEY_DOWN    kVK_DownArrow

#define KEY_BACK    kVK_Delete
#define KEY_TAB     kVK_Tab
#define KEY_RETURN  kVK_Return
#define KEY_SHIFT   kVK_Shift
#define KEY_CONTROL kVK_Control
#define KEY_MENU    kVK_Option
#define KEY_ESC     kVK_Escape
#define KEY_SPACE   kVK_Space
#define KEY_COMMAND kVK_Command


typedef struct {
	// /*NSApplication**/ void* app;
	// /*NSWindow**/ void* window;

	// /*id<MTLDevice>*/ void* device;
	// /*CAMetalLayer**/ void* metalLayer;
	// /*id<MTLCommandQueue>*/ void* commandQueue;
	// /*id<MTLRenderPipelineState>*/ void* pipeline;

	int2_t screenSize;
	int2_t framebufferSize;

	// /*id<MTLTexture>*/ void* framebufferTexture;
	u32* framebuffer;
	u32* scaledFramebuffer;

	vec2_t worldSpaceMin;
	vec2_t worldSpaceMax;
	vec2_t worldSpace;

	button_t keyboard[256];
	mouse_t mouse;
} video_t;

typedef struct {
	union {
		i16 channels[2];
		struct {
			i16 left;
			i16 right;
		};
	};
} sys_audio_sample_t;
typedef struct {
	int channels;
	int samplesPerSecond;
	int bytesPerSample;
	size_t sampleCount;
	sys_audio_sample_t data[];
} sys_wave_t;

typedef struct {
	sys_wave_t* wave;
	float cursor;
	float volume;
} sys_sound_t;

typedef void (*audio_mixer_proc)(void* outputStream, int sampleCount, void* userdata);

#define SYS_FUNC

SYS_FUNC void Sys_InitMetal();
SYS_FUNC void Sys_OutputFrameAndSync();
SYS_FUNC void Sys_InitWindow();
// SYS_FUNC void Sys_InitMetal();
SYS_FUNC void Sys_PollEvents();

SYS_FUNC void Sys_QueueSound(sys_wave_t* wave, float volume);
SYS_FUNC void Sys_InitAudio(audio_mixer_proc mixerProc);
SYS_FUNC sys_wave_t* Sys_LoadWave(allocator_t* allocator, file_data_t* fileData);
char* Sys_GetResourcePath(allocator_t* allocator, char* filename);

#endif
