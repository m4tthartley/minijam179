//
//  Created by Matt Hartley on 15/03/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

// #include <core/sysaudio.h>
#include <sys/mman.h>

#include "game.h"
// #include "system.h"
// #include "system_resource.c"

#define CORE_IMPL
#include <core/sys.h>
#include <core/sysvideo.h>
#include <core/core.h>
#include <core/math.h>
#include <core/hotreload.h>

programstate_t* program = NULL;

void InitSharedMemory() {
	char* shmName = "/game_shm";
	shm_unlink(shmName);

	int fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("shm_open");
		return;
	}

	if (ftruncate(fd, sizeof(programstate_t))) {
		perror("ftruncate");
		return;
	}

	program = mmap(0, sizeof(programstate_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (program == MAP_FAILED) {
		perror("mmap");
		return;
	}
}

pid_t pid;
void StartSubProcess(char* path) {
	pid = fork();
	if (!pid) {
		execl(path, path, NULL);
		perror("execl");
		exit(1);
	}
}

int main() {
	// programstate_t* program = sys_alloc_memory(sizeof(programstate_t));
	InitSharedMemory();
	sys_zero_memory(program, sizeof(programstate_t));

	char gameExePath[MAX_PATH_LENGTH];
	sys_get_bundle_path(gameExePath, MAX_PATH_LENGTH, "game");

	reload_init(gameExePath);

	StartSubProcess(gameExePath);

	while (TRUE) {
		if (reload_check_lib_modified()) {
			kill(pid, SIGTERM);
			waitpid(pid, NULL, 0);

			StartSubProcess(gameExePath);
		}

		usleep(16666);
	}
}


void finished() __attribute__((destructor));
void finished() {
	print("Goodbye.");
}
