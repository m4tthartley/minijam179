//
//  Main
//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

// #include "core/platform.h"
#include "game.h"
#include "system.h"

#include "system_resource.c"

#define CORE_IMPL
#include <core/core.h>
#include <core/math.h>
#include <core/hotreload.h>
// #include <core/platform.h>

extern sys_t sys;
extern video_t video;

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


// #ifdef HOTRELOAD
// hotreload_t hotreload;
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

	// G_Init();
	// hotreload_run("game");
	reload_register_state("sys", /*&sys, sizeof(sys)*/ NULL, sizeof(sys));
	reload_register_state("video", /*&video, sizeof(video)*/ NULL, sizeof(video));
	char* gameSoPath = Sys_GetResourcePath(NULL, "game.so");
	reload_init(gameSoPath);

	reload_run_func("G_Init", NULL);
	while (/*sys.running*/TRUE) {
		// if (hotreload.active && hotreload.reload()) {
		// 	return 0;
		// }
		reload_update();
		// S_Update();
		reload_run_func("G_Update", NULL);
	}
}
// #else
// int main() {
// 	if (!hotreload.active || hotreload.init) {
// 		S_Init();
// 	}
// 	while (sys.running) {
// 		if (hotreload.active && hotreload.reload()) {
// 			return 0;
// 		}
// 		S_Update();
// 	}
// }
// #endif

void finished() __attribute__((destructor));
void finished() {
	print("Goodbye.");
}
