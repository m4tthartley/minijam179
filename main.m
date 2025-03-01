//
//  Main
//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <core/core.h>

#include "game.h"
#include "system.h"

#define CORE_IMPL
#include <core/hotreload.h>

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
	G_Init();
	// hotreload_run("game");
	reload_register_state("sys", &sys, sizeof(sys));
	reload_register_state("video", &video, sizeof(video));
	reload_init("build/game");
	while (sys.running) {
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
