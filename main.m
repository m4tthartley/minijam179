//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "game.h"
// #include "system.h"
#include "system_resource.c"

#define CORE_IMPL
#include <core/sys.h>
#include <core/sysvideo.h>
#include <core/sysaudio.h>
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


// print_float
// fmt
void print_int(char* buf, int len, int32_t num) {
	// int i = 0;
	// while (num > 0) {
	// __builtin_clz()

	// }
	int ci = 0;
	if (num < 0) {
		buf[ci++] = '-';
		num = abs(num);
	}
	if (num != 0) {
		int l = log10(abs(num));
		int x = 0;
		for (int i=0; i<l+1; ++i) {
			buf[ci + (l-i)] = num%10 + '0';
			num /= 10;
		}
		ci += l+1;
	} else {
		buf[ci++] = '0';
	}
	buf[ci++] = 0;
}

int idiv10(int input) {
	uint32_t n = abs(input);
	uint64_t tmp = (uint64_t)n * 0xCCCCCCCD;
	uint32_t q = tmp >> 35;

	// uint32_t signMask = input & 0x80000000;
	// uint32_t asd = q | signMask;
	// int result = asd;
	// return result;

	int result = q;
	if (input & 0x80000000) {
		result = 0 - result;
	}
	return result;
}

// Max 32bit: 4,294,967,295
// Max 64bit: 18,446,744,073,709,551,615
int _log2to10_tbl[] = {
	0,0,0,0,1,1,1,2,2,2,3,3,3,3,
	4,4,4,5,5,5,6,6,6,6,7,7,7,8,8,8,9,9,
};
uint64_t _base10_tbl[] = {
	1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000,
	10000000000,100000000000,1000000000000,10000000000000,
	100000000000000,1000000000000000,10000000000000000,
	100000000000000000,1000000000000000000,//10000000000000000000,
};
int ilog10(int input) {
	int value = abs(input);
	int base2 = 31 - __builtin_clz(value | 1);
	int base10TblIndex = _log2to10_tbl[base2] + 1;
	if (value < _base10_tbl[base10TblIndex]) {
		--base10TblIndex;
	}
	int base10 = _base10_tbl[base10TblIndex];
	if (input & 0x80000000) {
		base10 = 0 - base10;
	}
	return base10;
}

int main() {
	// print("log %i", ilog10(99));
	// print("log %i", ilog10(100));
	// print("log %i", ilog10(101));
	// print("log %i", ilog10(120));

	// print("log %i", ilog10(999));
	// print("log %i", ilog10(1000));
	// print("log %i", ilog10(1001));

	int NUM = 15;
	FOR (i, 11) {
		print("log %i = %i", NUM, ilog10(NUM));
		NUM *= 10;
	}

	// FOR (i, 32) {
	// 	uint32_t num = 1 << i;
	// 	// print("bit %i, log %i, %i", i, (int)log10(num), num);
	// 	print_inline("%u,", (uint32_t)log10(num));
	// }
	// print("\n");

	// uint64_t bigNum = 1;
	// while (/*bigNum < 0xFFFFFFFF /*FFFFFFFF*/ TRUE) {
	// 	print("Num: %llu", bigNum);
	// 	if (bigNum * 10 < bigNum) {
	// 		break;
	// 	}
	// 	bigNum *= 10;
	// }

	char buffer[64];
	print_int(buffer, 64, 55);
	print(buffer);

	print_int(buffer, 64, 7);
	print(buffer);

	print_int(buffer, 64, 0);
	print(buffer);

	print_int(buffer, 64, 255);
	print(buffer);

	print_int(buffer, 64, -255);
	print(buffer);

	uint32_t num = 237;
	uint64_t tmp = (uint64_t)num * 0xCCCCCCCD;
	uint32_t q = tmp >> 35;
	// result *= 10;

	int a = idiv10(255);
	int b = idiv10(-255);
	int c = idiv10(275443);
	int d = idiv10(-17);
	int e = idiv10(0);

	escape_color_bg(escape_256_color(2, 5, 2));
	print("\n [Green Energy] \n");

	// escape_color(escape_basic_color(ESCAPE_RED, _True));
	// print(" [Basic Color] ");

	// escape_color(escape_256_color(2, 4, 5));
	// escape_mode(ESCAPE_BOLD | ESCAPE_STRIKETHROUGH | ESCAPE_ITALIC | ESCAPE_INVERTED);
	// print(" [Hello World] ");

	// escape_mode(ESCAPE_RESET);
	// print(" Did the reset work? ");

	// escape_color(escape_256_color(5, 1, 1));
	// escape_mode(ESCAPE_BOLD | ESCAPE_INVERTED);
	// print(" An error has occurred! ");

	// print("Player position: @i, %i")

	escape_mode(ESCAPE_RESET);

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
