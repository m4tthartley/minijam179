//
//  Created by Matt Hartley on 15/03/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//


#include <core/core.h>

#include "game.h"

programstate_t* program = NULL;

void InitSharedMemory() {
	char* shmName = "/game_shm";
	int fd = shm_open(shmName, O_RDWR, 0666);
	if (fd < 0) {
		perror("shm_open");
		return;
	}

	program = mmap(0, sizeof(programstate_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (program == MAP_FAILED) {
		perror("mmap");
		return;
	}
}

int main() {
	print("GAME");

	InitSharedMemory();

	sys_init_window(&program->window, "Green Energy", 1280, 800, WINDOW_CENTERED);
	program->video.screenSize = int2(1280, 800);
	Sys_InitMetal(&program->window, &program->video);

	print("running %i", program->game.running);

	if (!program->initialized) {
		G_Init(program);
	}

	program->initialized = _True;

	while (TRUE) {
		sys_poll_events(&program->window);

		G_Update(program);

		Sys_OutputFrameAndSync(&program->window, &program->video);
	}
}
