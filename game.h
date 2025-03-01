//
//  Created by Matt Hartley on 23/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#ifndef __GAME_H__
#define __GAME_H__

#include <core/core.h>
#include "system.h"
#include "bitmap.h"

typedef enum {
	TILE_NONE,
	TILE_GRASS,
	TILE_WATER,
} tile_type_t;
typedef struct {
	int height;
} tile_t;

#define MAP_SIZE 64

typedef struct {
	u8 objc_state[128];

	b32 running;
	allocator_t assetMemory;
	allocator_t scratchBuffer;

	bitmap_t* testBitmap;
	bitmap_t* fontBitmap;
	bitmap_t* mapBitmap;
	sys_wave_t* pianoTest;

	tile_t map[MAP_SIZE*MAP_SIZE];
	vec2_t cameraPos;
} sys_t;

void G_Init();
void G_Update();

#endif
