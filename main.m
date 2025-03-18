//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <sys/mman.h>

#include "game.h"
// #include "system.h"
#include "system_resource.c"

#define CORE_IMPL
#include <core/sys.h>
#include <core/sysvideo.h>
#include <core/sysaudio.h>
#include <core/terminal.h>
#include <core/core.h>
#include <core/math.h>
#include <core/hotreload.h>

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


sys_window_t window;
// sysaudio_t audio;

enum {
	ESCAPE_BLACK = 0,
	ESCAPE_RED = 1,
	ESCAPE_GREEN = 2,
	ESCAPE_YELLOW = 3,
	ESCAPE_BLUE = 4,
	ESCAPE_MAGENTA = 5,
	ESCAPE_CYAN = 6,
	ESCAPE_WHITE = 7,
};

enum {
	ESCAPE_NONE = (1<<0),
	ESCAPE_BOLD = (1<<1),
	ESCAPE_DIM = (1<<2),
	ESCAPE_ITALIC = (1<<3),
	ESCAPE_UNDERLINE = (1<<4),
	ESCAPE_BLINK = (1<<5),
	ESCAPE_INVERTED = (1<<7),
	ESCAPE_HIDDEN = (1<<8),
	ESCAPE_STRIKETHROUGH = (1<<9),
	ESCAPE_END = (1<<10),
};

ecstr_t escape_code_asd(int color, _Bool bright, _Bool bg, _Bool bold, _Bool underlined) {
	ecstr_t result = {0};

	int colorCode = 30;
	if (bright) {
		colorCode += 60;
	}
	if (bg) {
		colorCode += 10;
	}
	snprintf(result.s, 32, "\x1B[%i;%i;%im", (int)bold, underlined?4:0, colorCode+color);

	return result;
}

uint8_t escape_basic_color(uint8_t color, _Bool bright) {

}

uint8_t escape_256_color(uint8_t r, uint8_t g, uint8_t b) {
	return 16 + (r*36) + (g*6) + (b);
}

ecstr_t escape_code_old(int fgColor, int bgColor, _Bool bright, _Bool bold, _Bool underlined) {
	ecstr_t result = {0};

	int fgColorCode = 30;
	int bgColorCode = 40;
	if (bright) {
		fgColorCode += 60;
		bgColorCode += 60;
	}

	snprintf(result.s, 32, "\x1B[%i;%i;%i;%im", (int)bold, underlined?4:0, fgColorCode+fgColor, bgColorCode+bgColor);

	return result;
}

void escape_code(uint8_t fgColor, uint8_t bgColor, int options) {

	char buffer[64];
	snprintf(buffer, 64, "\x1B[%i;5;%im\x1B[%i;5;%im", fgColor>=1?38:39, fgColor, bgColor>=1?48:49, bgColor);
	FOR (i, 32) {
		if (options & (1 << i)) {
			snprintf(buffer+strlen(buffer), 64-strlen(buffer), "\x1B[%im", i);
		}
	}
	
	print(buffer);
}


int main() {
	escape_code(1, -1, ESCAPE_BOLD);
	print(" [Hello World] ");

	// ecstr_t ec = escape_code_old(1, 0, 0, _True, _False);
	// char* ecstr = ec.s;
	// print(escape_code_old(1, 7, _False, _False, _False).s);
	// print(" [Hello World] ");
	// print(escape_code_old(1, 7, _True, _False, _False).s);
	// print(" [Hello World] ");
	// print(escape_code_old(2, 0, 1, _True, _True).s);
	// print(TERM_RESET);
	// print("\x1B[38;5;093m");
	// print("\x1B[48;5;198m");
	// print("HELLO WORLD");

	// FOR (i, 256) {
	// 	print_inline("\x1B[48;5;%im %i ", i, i);
	// }

	exit(0);

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

	// programstate_t* program = sys_alloc_memory(sizeof(programstate_t));
	// sys_zero_memory(program, sizeof(programstate_t));

	char* gameSoPath = Sys_GetResourcePath(NULL, "game.so");
	reload_init(gameSoPath);
	reload_register_state("game", NULL, sizeof(gamestate_t));
	reload_register_state("video", NULL, sizeof(video_t));
	reload_register_state("audio", NULL, sizeof(sysaudio_t));

	video_t* video = reload_get_state_ptr("video");
	sysaudio_t* audio = reload_get_state_ptr("audio");
	sys_init_window(&window, "Green Energy", 1280, 800, WINDOW_CENTERED);
	video->screenSize = int2(1280, 800);
	Sys_InitMetal(&window, video);
	sys_init_audio(audio, SYSAUDIO_DEFAULT_SPEC);
	sys_start_audio(audio);

	reload_run_func("G_Init", &window);
	// G_Init(program);
	while (TRUE) {
		if (reload_check_lib_modified()) {
			sys_set_audio_callback(audio, NULL);
			sys_stop_audio(audio);
			reload_reload();
			video = reload_get_state_ptr("video");
			audio = reload_get_state_ptr("audio");
			sys_start_audio(audio);
		}

		sys_poll_events(&window);

		reload_run_func("G_Update", &window);

		Sys_OutputFrameAndSync(&window, video);
	}
}

// programstate_t* program = NULL;

// void InitSharedMemory() {
// 	char* shmName = "/game_shm";
// 	shm_unlink(shmName);

// 	int fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
// 	if (fd < 0) {
// 		perror("shm_open");
// 		return;
// 	}

// 	if (ftruncate(fd, sizeof(programstate_t))) {
// 		perror("ftruncate");
// 		return;
// 	}

// 	program = mmap(0, sizeof(programstate_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// 	if (program == MAP_FAILED) {
// 		perror("mmap");
// 		return;
// 	}
// }

// pid_t pid;
// void StartSubProcess(char* path) {
// 	pid = fork();
// 	if (!pid) {
// 		execl(path, path, NULL);
// 		perror("execl");
// 		exit(1);
// 	}
// }

// int main() {
// 	// programstate_t* program = sys_alloc_memory(sizeof(programstate_t));
// 	InitSharedMemory();
// 	sys_zero_memory(program, sizeof(programstate_t));

// 	char gameExePath[MAX_PATH_LENGTH];
// 	sys_get_bundle_path(gameExePath, MAX_PATH_LENGTH, "game");

// 	reload_init(gameExePath);

// 	StartSubProcess(gameExePath);

// 	while (TRUE) {
// 		if (reload_check_lib_modified()) {
// 			kill(pid, SIGTERM);
// 			waitpid(pid, NULL, 0);

// 			StartSubProcess(gameExePath);
// 		}

// 		usleep(16666);
// 	}
// }


void finished() __attribute__((destructor));
void finished() {
	print("Goodbye.");
}
