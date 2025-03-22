//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <math.h>
#include <os/clock.h>
#include <mach/mach_time.h>
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
#include <core/imath.h>
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

/*
	fmtstr
	format_string
*/

void print_int(char* buf, int len, int32_t num) {
	int ci = 0;
	if (num < 0) {
		buf[ci++] = '-';
		num = abs(num);
	}
	if (num != 0) {
		int l = ilog10(num);
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

// 86763ll
// 56399ll
// 22134ll
// 164715ll
typedef struct {
	uint64_t startTime;
	uint64_t endTime;
	uint64_t duration;
} perftimer_t;
perftimer_t perf_timer_start() {
	__asm__ __volatile__ ("isb");
	uint64_t time = mach_absolute_time();
	return (perftimer_t){
		.startTime = time,
	};
}
void perf_timer_end(perftimer_t* timer) {
	__asm__ __volatile__ ("isb");
	uint64_t time = mach_absolute_time();
	timer->endTime = time;
	timer->duration = timer->endTime - timer->startTime;
}

#define VALUE_COUNT 10000000
int values[VALUE_COUNT];

void MyLogPerfTest() {
	perftimer_t timer = perf_timer_start();
	int counter = 0;
	FOR (i, VALUE_COUNT) {
		// print_int(buffer, 64, 25565);
		values[i] = ilog10(counter);
		++counter;
	}
	perf_timer_end(&timer);
	print_inline("my log time: %ull", timer.duration);
	int random = randr(0, VALUE_COUNT);
	print("  random value at index %i = %i", random, values[random]);
}

void CLogPerfTest() {
	perftimer_t timer = perf_timer_start();
	int counter = 0;
	FOR (i, VALUE_COUNT) {
		// print_int_slow(buffer, 64, 25565);
		values[i] = log10f(counter);
		++counter;
	}
	perf_timer_end(&timer);
	print_inline(" C log time: %ull", timer.duration);
	int random = randr(0, VALUE_COUNT);
	print("  random value at index %i = %i", random, values[random]);
}

void MyDivPerfTest() {
	perftimer_t timer = perf_timer_start();
	FOR (i, VALUE_COUNT) {
		values[i] = idiv10(25565);
	}
	perf_timer_end(&timer);
	print("my div time: %ull", timer.duration);
}

void CDivPerfTest() {
	perftimer_t timer = perf_timer_start();
	FOR (i, VALUE_COUNT) {
		values[i] = 25565 / 10;
	}
	perf_timer_end(&timer);
	print(" C div time: %ull", timer.duration);
}

void PerformanceTesting() {
// #	define VALUE_COUNT 1000000
// 	int values[VALUE_COUNT];
	perftimer_t timer;
	int counter = 0;

	MyLogPerfTest();
	CLogPerfTest();
	
	MyDivPerfTest();
	CDivPerfTest();
}

int main() {
	PerformanceTesting();
	exit(0);

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

	int a = imod10(255);
	int b = imod10(-255);
	int c = imod10(275443);
	int d = imod10(-17);
	int e = imod10(0);

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
