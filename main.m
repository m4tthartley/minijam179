//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <limits.h>
#include <math.h>
#include <os/clock.h>
#include <mach/mach_time.h>
#include <stdlib.h>
#include <sys/mman.h>

#define CORE_IMPL
#include <core/sys.h>
#include <core/sysvideo.h>
#include <core/sysaudio.h>
#include <core/core.h>
#include <core/math.h>
#include <core/imath.h>
#include <core/hotreload.h>
#include <core/print.h>
#undef CORE_IMPL

#include "game.h"
// #include "system.h"
#include "system_resource.c"


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
	print("my log time: %ull", timer.duration);
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
	print(" C log time: %ull", timer.duration);
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


/*
	fmtstr
	format_string
*/

#define print_int sprint_int
#define print_float sprint_float

void TestFormatting() {
	for (int i=0; i<100; ++i) {
		int testInt = randr(0, 0x7FFFFFFF) - 0x7FFFFFFF/2;
		char mine[16];
		char libc[16];
		print_int(mine, 16, testInt);
		snprintf(libc, 16, "%i", testInt);
		if (str_compare(mine, libc)) {
			escape_color(escape_256_color(1, 4, 1));
			escape_mode(ESCAPE_INVERTED);
			print(" MATCH [%s] [%s] ", mine, libc);
		} else {
			escape_color(escape_256_color(5, 1, 1));
			escape_mode(ESCAPE_INVERTED);
			print(" MISMATCH [%s] - [%s] ", mine, libc);
		}
	}

	for (int i=0; i<100; ++i) {
		float testFloat = randfr(-0xFFFF, 0xFFFF);
		char mine[16];
		char libc[16];
		print_float(mine, 16, testFloat, 3);
		snprintf(libc, 16, "%.3f", testFloat);
		if (str_compare(mine, libc)) {
			escape_color(escape_256_color(1, 4, 1));
			escape_mode(ESCAPE_INVERTED);
			print(" MATCH [%s] [%s] ", mine, libc);
		} else {
			escape_color(escape_256_color(5, 1, 1));
			escape_mode(ESCAPE_INVERTED);
			print(" MISMATCH [%s] - [%s] ", mine, libc);
		}
	}
}


int main() {
	char buffer[64];

	sprint(buffer, sizeof(buffer), "int: %i, u32: %u \n", -556, 0xFFFFFFFF);
	print(buffer);
	sprint(buffer, sizeof(buffer), "i64: %li, u64: %lu \n", -556, (uint64_t)0xFFFFFFFFFFFF);
	print(buffer);
	sprint(buffer, sizeof(buffer), "float: %f \n", 255.123456789);
	print(buffer);

	// int age = 30;
	// char* text = "Matt Hartley";
	// sprint(buffer, sizeof(buffer), "my name is %s, and my age is %i", text, age);
	// print(buffer);

	// sprint(buffer, sizeof(buffer), "The number is %i, do you like it?\nhow about this float %f", 235, 5.0f);
	// print(buffer);
	// sprint(buffer, sizeof(buffer), "The number is %i, do you like it?\nhow about this float %f", 345321, -5.7f);
	// print(buffer);
	// sprint(buffer, sizeof(buffer), "The number is %i, do you like it?\nhow about this float %f", -17, 255.455f);
	// print(buffer);
	// sprint(buffer, sizeof(buffer), "The number is %i, do you like it?\nhow about this float %f", -2334543, -500.12f);
	// print(buffer);

	// print_float(buffer, 64, 134.255, 4);
	// print(buffer);
	// print_float(buffer, 64, -345345.245345, 4);
	// print(buffer);
	// print_float(buffer, 64, -34.245345, 4);
	// print(buffer);

	// print_int(buffer, 64, 55);
	// print(buffer);

	// print_int(buffer, 64, 7);
	// print(buffer);

	// print_int(buffer, 64, 0);
	// print(buffer);

	// print_int(buffer, 64, 255);
	// print(buffer);

	// print_int(buffer, 64, -255);
	// print(buffer);

	// uint32_t num = 237;
	// uint64_t tmp = (uint64_t)num * 0xCCCCCCCD;
	// uint32_t q = tmp >> 35;

	// int a = imod10(255);
	// int b = imod10(-255);
	// int c = imod10(275443);
	// int d = imod10(-17);
	// int e = imod10(0);

	// escape_color_bg(escape_256_color(5, 1, 1));
	// escape_mode(ESCAPE_INVERTED);
	// \x1B[38;5;203m
	// \x1B[7m
	// sys_print("\x1B[38;5;203;7m");
	escape_color(escape_256_color(1, 4, 2));
	escape_mode(ESCAPE_INVERTED);
	print("\n [Green Energy] \n");

	escape_mode(ESCAPE_RESET);


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
