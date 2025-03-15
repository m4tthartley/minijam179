//
//  Main
//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <core/sys.h>
#include <core/core.h>
#include <core/math.h>
#include <core/sysaudio.h>
#include <core/hotreload.h>

#include "game.h"
#include "system.h"
#include "system_resource.c"

#define CORE_IMPL
#include <core/sysvideo.h>

// extern gamestate_t game;
// extern video_t video;

// int main() {
// 	print_error("Hello JAM");
// 	V_Init();
// }

#ifdef RELOADABLE
// int main() {
// 	// S_Init();
// 	reload_load();
// 	reload_func_ptr_t initFunc = reload_load_func("S_Init");
// 	reload_register_state("video", video);
// 	while (running) {
// 		reload_check();
// 		S_Update();
// 	}
// 	exit(0);
// }
#else
// int main() {
// 	S_Init();
// 	while (sys.running) {
// 		S_Update();
// 	}
// 	exit(0);
// }
#endif


int main() {
#ifdef __RELEASE__
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
	char bundlePath[MAX_PATH_LENGTH];
	CFURLGetFileSystemRepresentation(bundleUrl, true, (UInt8*)bundlePath, MAX_PATH_LENGTH);
	print("Bundle path: %s", bundlePath);
	char logPath[MAX_PATH_LENGTH];
	copy_memory(logPath, bundlePath, MAX_PATH_LENGTH);
	char_append(logPath, "/stdout.log", MAX_PATH_LENGTH);
	FILE* stdoutFile = freopen(logPath, "w", stdout);
	// FILE* stderrFile = freopen(logPath, "w", stderr);
#endif
	Sys_GetResourcePath(NULL, "stdout.log");

	programstate_t* program = sys_alloc_memory(sizeof(programstate_t));
	sys_zero_memory(program, sizeof(programstate_t));
	
	sys_init_window(&program->window, "Green Energy", 1280, 800, WINDOW_CENTERED);
	program->video.screenSize = int2(1280, 800);
	Sys_InitMetal(&program->window, &program->video);

	reload_register_state("game", NULL, sizeof(gamestate_t*));
	reload_register_state("video", NULL, sizeof(video_t*));
	char* gameSoPath = Sys_GetResourcePath(NULL, "game.so");
	reload_init(gameSoPath);

	// reload_run_func("G_Init", program);
	G_Init(program);
	while (TRUE) {
		reload_update();
		
		sys_poll_events(&program->window);

		reload_run_func("G_Update", program);

		Sys_OutputFrameAndSync(&program->window, &program->video);
	}
}


void finished() __attribute__((destructor));
void finished() {
	print("Goodbye.");
}
