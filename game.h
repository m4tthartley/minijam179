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
typedef enum {
	TILE_BUILDING_NONE,
	TILE_BUILDING_TREE,
	TILE_BUILDING_TOWER,
} tile_building_type_t;
typedef struct {
	tile_building_type_t type;
	int varient;
} tile_building_t;
typedef struct {
	tile_type_t type;
	tile_building_t building;
	int2_t pos;
	int height;
} tile_t;

typedef enum {
	ENTITY_NONE,
	ENTITY_WORKER,
	ENTITY_BEAR,
} entity_type_t;
typedef struct {
	entity_type_t type;
	vec2_t pos;
	int2_t tilePos;
	int2_t tileDest;
	float aniFrame;
} entity_t;

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
	entity_t entities[64];
	vec2_t cameraPos;
} sys_t;

void G_Init();
void G_Update();

#endif
