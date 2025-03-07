//
//  Created by Matt Hartley on 23/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include "game.h"
#include "render.h"
#include "bitmap.h"
#include "font.c"
#include "system.h"

#include <core/core.c>
#define CORE_IMPL
#include <core/core.h>
#include "core/platform.h"
#include <core/math.h>

sys_t sys;
extern video_t video;

file_data_t* ReadEntireFile(allocator_t* allocator, char* filename) {
	file_data_t* result = NULL;

	file_t file = file_open(filename);
	if (file) {
		stat_t info = file_stat(file);
		result = alloc_memory(allocator, sizeof(file_data_t)+info.size);
		copy_memory(result, &info, sizeof(info));
		file_read(file, 0, result+1, info.size);
		file_close(file);
	}

	return result;
}

vec2_t G_TileSpaceToScreenSpace(vec2_t pos, float height) {
	vec2_t result = vec2(
		((float)pos.x*1.0f)+((float)pos.y*1.0f),
		((float)pos.y*0.5f)-((float)pos.x*0.5f)+(height*0.5f)
	);
	return result;
}

vec2_t G_ScreenSpaceToTileSpace(vec2_t pos) {
	vec2_t result = {
		// (pos.x * 0.5f) - pos.y ,
		// (pos.y + (pos.x * 0.5f))
		pos.x - pos.y,
		pos.y*2.0f + pos.x*2.0f
	};
	return result;
}

b32 G_PointOnTile(tile_t* tile, vec2_t point) {
	point = add2(point, sys.cameraPos);
	float height = (float)tile->height;
	if ((point.x*0.5f) - (point.y - (height*0.5f)) > tile->pos.x &&
		(point.x*0.5f) - (point.y - (height*0.5f)) < (tile->pos.x+1) &&
		(point.x*0.5f) + (point.y - (height*0.5f)) > tile->pos.y &&
		(point.x*0.5f) + (point.y - (height*0.5f)) < (tile->pos.y+1)) {
		return TRUE;
	}
	return FALSE;
}

void G_GenTower(int2_t pos, float energy) {
	if (pos.x < 0 || pos.x >= MAP_SIZE ||
		pos.y < 0 || pos.y >= MAP_SIZE) {
		return;
	}
	sys.map[pos.y*MAP_SIZE+pos.x].building.type = TILE_BUILDING_TOWER;
	sys.map[pos.y*MAP_SIZE+pos.x].building.varient = randr(0, 3);
	sys.map[pos.y*MAP_SIZE+pos.x].building.health = 1.0f;

	if (energy > 0.0f) {
		G_GenTower(int2(pos.x+1, pos.y), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x-1, pos.y), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x, pos.y+1), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x, pos.y-1), energy - (0.5f + randf()*0.5f));
	}
}

void G_Init() {
	sys.running = TRUE;
	Sys_InitWindow();
	Sys_InitMetal();

	Sys_InitAudio(NULL);

	float aspect = (float)video.framebufferSize.x / (float)video.framebufferSize.y;
	video.worldSpaceMin = (vec2_t){-10.0f * aspect, -10.0f};
	video.worldSpaceMax = (vec2_t){10.0f * aspect, 10.0f};
	video.worldSpace = (vec2_t){(float)video.framebufferSize.x / 8.0f, (float)video.framebufferSize.y / 8.0f};

	sys.assetMemory = virtual_heap_allocator(MB(10), NULL);
	sys.scratchBuffer = virtual_bump_allocator(MB(1), NULL);

	sys.testBitmap = LoadBitmap(&sys.assetMemory, Sys_GetResourcePath(NULL, "assets/test.bmp"));
	sys.mapBitmap = LoadBitmap(&sys.assetMemory, Sys_GetResourcePath(NULL, "assets/map.bmp"));

	sys.fontBitmap = Fnt_GenBitmap(&sys.assetMemory, &FONT_DEFAULT);

	// sys.pianoTest = Sys_LoadWave(&sys.assetMemory, ReadEntireFile(&sys.assetMemory, "/Users/matt/Desktop/piano.wav"));
	// Sys_QueueSound(sys.pianoTest, 0.5f);

	FOR (y, MAP_SIZE)
	FOR (x, MAP_SIZE) {
		float height = fbm(vec2((float)x*0.05f + 1.0f, (float)y*0.05f + 1.0f)) * 40.0f;
		sys.map[y*MAP_SIZE+x].pos = int2(x, y);
		sys.map[y*MAP_SIZE+x].height = height;

		if (rand2d(vec2(x, y)) > 0.9f) {
			sys.map[y*MAP_SIZE+x].building.type = TILE_BUILDING_TREE;
			sys.map[y*MAP_SIZE+x].building.health = 1.0f;
		}

		// float buildingChance = fbm(vec2((float)x*0.2f + 123456.0f, (float)y*0.2f + 123456.0f));
		// if (buildingChance > 0.7f) {
		// 	sys.map[y*MAP_SIZE+x].buildingType = TILE_BUILDING_TOWER;
		// }
	}
	
	print("generated towers...");
	FOR (i, 10) {
		G_GenTower(int2(randr(0, MAP_SIZE), randr(0, MAP_SIZE)), randfr(1.0f, 2.0f));
	}
	print("done.");
}

vec2_t boxPos = {4, 0};
vec2_t boxSpeed = {1, 1};

R_FUNC void R_BlitFontBitmaps(bitmap_t* bitmap, font_text_t* text, vec2_t pos) {
	FOR (i, text->numChars) {
		font_text_char_t* c = text->chars + i;
		R_BlitBitmapAtlas(
			bitmap,
			c->index%16 * 8,
			c->index/16 * 8,
			8,
			8,
			add2(pos, c->pos)
		);
	}
}

tile_t* G_GetTile(int2_t pos) {
	return sys.map + (pos.y*MAP_SIZE + pos.x);
}

void G_AddEntity(entity_type_t type, int2_t pos) {
	FOR (i, array_size(sys.entities)) {
		if (sys.entities[i].type == ENTITY_NONE) {
			sys.entities[i] = (entity_t){
				.type = type,
				.tilePos = pos,
				.nextTileDest = pos,
				.tileDest = pos,
			};
			return;
		}
	}
	print("ENTITY ARRAY FULL");
}

int2_t pathDirs[] = {
	{0, 0},
	{-1, 0},
	{0, -1},
	{1, 0},
	{0, 1},
};
typedef struct {
	// 1st bit is 1 if the node has been searched
	// The rest of the bits are the parent direction
	// 0=NULL, 1=left, 2=bottom, 3=right, 4=top
	u8 data;
} path_node_t;
typedef struct {
	u8 x, y;
	u8 data;
	u8 depth;
} path_queue_node_t;
typedef struct {
	int2_t* tiles;
	int length;
} path_t;
path_node_t pathNodes[MAP_SIZE*MAP_SIZE];
path_queue_node_t pathQueue[MAP_SIZE*MAP_SIZE];
int pathQueueCount = 0;
int pathQueueQueued = 0;
path_t G_GetNextTileFromPath(int2_t pos, path_queue_node_t node) {
	// print("Found destination");
	// print("pos %i,%i", pos.x, pos.y);

	if (!node.depth) {
		return (path_t){
			.tiles = &pos,
			.length = 1,
		};
	}

	path_t path = {
		.tiles = push_memory(&sys.scratchBuffer, sizeof(int2_t)*node.depth),
		.length = node.depth,
	};

	// Unroll
	int2_t previous = int2(node.x, node.y);
	int2_t walkBack = int2(node.x, node.y);
	int index = 0;
	while ((walkBack.x != pos.x || walkBack.y != pos.y) && index<1000) {
		// print("tile memory index: %i", node.depth-1 - index);
		path.tiles[node.depth-1 - index] = walkBack;

		int2_t dir = pathDirs[(pathNodes[walkBack.y*MAP_SIZE+walkBack.x].data>>1)];
		// print("walk back: %i,%i; dir %i,%i", walkBack.x, walkBack.y, dir.x, dir.y);
		previous = walkBack;
		walkBack.x = walkBack.x - dir.x;
		walkBack.y = walkBack.y - dir.y;
		++index;
	}
	// print("got it?");
	// print("walk back: %i,%i;", walkBack.x, walkBack.y);
	assert(previous.x >= pos.x-1 &&
			previous.x <= pos.x+1 &&
			previous.y >= pos.y-1 &&
			previous.y <= pos.y+1);

	if (path.tiles[0].large == int2(0,0).large) {
		int x = 0;
	}

	return path;
}
path_t G_PathFind(int2_t pos, int2_t dest) {
	// print("G_PathFind");
	zero_memory(pathNodes, sizeof(pathNodes));
	pathQueueCount = 0;
	pathQueueQueued = 0;

	pathQueue[pathQueueCount++] = (path_queue_node_t){
		.x = pos.x,
		.y = pos.y,
		.data = 0,
		.depth = 0,
	};
	++pathQueueQueued;

	while (pathQueueQueued) {
		path_queue_node_t node = pathQueue[pathQueueCount-pathQueueQueued];
		// print("path node %i,%i", node.x, node.y);
		--pathQueueQueued;
		if (node.x==dest.x && node.y==dest.y) {
			return G_GetNextTileFromPath(pos, node);
		}

		FOR (i, 4) {
			int2_t dir = pathDirs[i+1];
			int2_t tile = int2(node.x+dir.x, node.y+dir.y);
			if (tile.large == dest.large &&
				sys.map[tile.y*MAP_SIZE+tile.x].building.type != TILE_BUILDING_NONE) {
				return G_GetNextTileFromPath(pos, node);
			}

			if (tile.x >= 0 &&
				tile.x < MAP_SIZE &&
				tile.y >= 0 &&
				tile.y < MAP_SIZE &&
				sys.map[tile.y*MAP_SIZE+tile.x].building.type == TILE_BUILDING_NONE) {
				if (!(pathNodes[tile.y*MAP_SIZE+tile.x].data & 0x1)) {
					pathNodes[tile.y*MAP_SIZE+tile.x].data |= 0x1;
					pathNodes[tile.y*MAP_SIZE+tile.x].data |= (i+1)<<1;

					pathQueue[pathQueueCount++] = (path_queue_node_t){
						.x = tile.x,
						.y = tile.y,
						.data = (i+1)<<1,
						.depth = node.depth + 1,
					};
					++pathQueueQueued;
				}
			}
		}
	}

	print("Path was not calculated");
	int2_t* pathMem = push_memory(&sys.scratchBuffer, sizeof(pos));
	copy_memory(pathMem, &pos, sizeof(pos));
	return (path_t){
		.tiles = pathMem,
		.length = 1,
	};
}

void G_Update() {
	Sys_PollEvents();
	vec2_t cameraSpeed = {0};
	cameraSpeed.x = (float)video.keyboard[KEY_D].down - (float)video.keyboard[KEY_A].down;
	cameraSpeed.y = (float)video.keyboard[KEY_W].down - (float)video.keyboard[KEY_S].down;
	sys.cameraPos = add2(sys.cameraPos, mul2f(cameraSpeed, 0.25f));
	if (video.keyboard[KEY_S].down) {
		int x = 0;
	}

	tile_t* highlightedTile = NULL;

	vec2_t mouseInWorldSpace = vec2(
		((float)video.mouse.pos.x/video.screenSize.x - 0.5f) * video.worldSpace.x,
		((float)video.mouse.pos.y/video.screenSize.y - 0.5f) * video.worldSpace.y
	);
	mouseInWorldSpace = sub2(mouseInWorldSpace, vec2(0.0f, 1.5f));
	vec2_t mouseTile = G_ScreenSpaceToTileSpace(sub2(mouseInWorldSpace, sys.cameraPos));
	
	R_DrawQuad(vec2(-video.worldSpace.x/2, -video.worldSpace.y/2), vec2(video.worldSpace.x, video.worldSpace.y), 0xFF000000);

	// R_BlitBitmap(sys.mapBitmap, vec2(0, 0));
	for (int y=MAP_SIZE-1; y>-1; --y)
	FOR (x, MAP_SIZE) {
		tile_t* tile = sys.map + (y*MAP_SIZE+x);
		// vec2_t pos = vec2(
		// 	((float)x*1.0f)+((float)y*1.0f),
		// 	((float)y*0.5f)-((float)x*0.5f)+((float)tile->height*0.5f)
		// );

		b32 highlighted = G_PointOnTile(tile, mouseInWorldSpace);
		if (highlighted) {
			highlightedTile = tile;
		}

		if (tile->building.type != TILE_BUILDING_NONE) {
			if (tile->building.health <= 0.0f) {
				tile->building.type = TILE_BUILDING_NONE;
				sys.wood += 5;
			}
		}
		
		vec2_t pos = G_TileSpaceToScreenSpace(vec2(x, y), tile->height);
		R_BlitBitmapAtlas(sys.mapBitmap, 0, 0, 16, 16, sub2(pos, sys.cameraPos));
		// if (highlighted) {
		// 	R_BlitBitmapAtlas(sys.mapBitmap, 16, 0, 16, 16, sub2(pos, sys.cameraPos));
		// }

		if (tile->building.type == TILE_BUILDING_TREE) {
			R_BlitBitmapAtlas(sys.mapBitmap, 48+32, 0, 16, 32, sub2(pos, sys.cameraPos));
		}
		if (tile->building.type == TILE_BUILDING_TOWER) {
			// R_BlitBitmapAtlas(sys.mapBitmap, 48+32, 0, 16, 32, sub2(pos, sys.cameraPos));
			R_BlitBitmapAtlas(sys.mapBitmap, 80+(16*tile->building.varient), 32, 16, 48, sub2(pos, sys.cameraPos));
		}
	}

	// Highlight tile
	if (highlightedTile) {
		if (video.keyboard[KEY_E].released) {
			G_AddEntity(ENTITY_WORKER, highlightedTile->pos);
		}

		if (video.mouse.left.pressed) {
			FOR (i, array_size(sys.entities)) {
				if (sys.entities[i].tilePos.large == highlightedTile->pos.large) {
					sys.workerDragMode = TRUE;
					sys.selectedWorker = sys.entities + i;
					break;
				}
			}
		}
		if (video.mouse.left.released) {
			if (sys.workerDragMode && sys.selectedWorker) {
				sys.selectedWorker->tileDest = highlightedTile->pos;
				sys.selectedWorker->job = highlightedTile->pos;
				sys.workerDragMode = FALSE;
			}
		}

		if (sys.workerDragMode && sys.selectedWorker) {
			path_t path = G_PathFind(sys.selectedWorker->tilePos, highlightedTile->pos);
			FOR (i, path.length) {
				tile_t* tile = &sys.map[path.tiles[i].y*MAP_SIZE+path.tiles[i].x];
				vec2_t pos = G_TileSpaceToScreenSpace(vec2(tile->pos.x, tile->pos.y), tile->height);
				R_BlitBitmapAtlas(sys.mapBitmap, 16, 0, 16, 16, sub2(pos, sys.cameraPos));
			}
		} else {
			vec2_t pos = G_TileSpaceToScreenSpace(vec2(highlightedTile->pos.x, highlightedTile->pos.y), highlightedTile->height);
			R_BlitBitmapAtlas(sys.mapBitmap, 16, 0, 16, 16, sub2(pos, sys.cameraPos));
		}
	}

	// if (!video.mouse.left.down) {
	// 	sys.workerDragMode = FALSE;
	// }

	// Entities
	FOR (i, array_size(sys.entities)) {
		entity_t* entity = sys.entities + i;
		if (entity->type == ENTITY_WORKER) {
			if (entity->tilePos.x != entity->nextTileDest.x || entity->tilePos.y != entity->nextTileDest.y) {
				entity->aniMove += 0.05f;
				entity->aniFrame += 0.1f;
				if (entity->aniFrame > 3.0f) {
					entity->aniFrame -= 3.0f;
				}
				if (entity->aniMove > 0.9f) {
					entity->aniMove = 0.0f;
					entity->tilePos = entity->nextTileDest;
				}
			} else {
				if (entity->tilePos.large != entity->tileDest.large) {
					path_t path = G_PathFind(entity->tilePos, entity->tileDest);

					// FOR (p, path.length) {
					// 	print_inline("<- (%i,%i)", path.tiles[p].x, path.tiles[p].y);
					// }
					// print_inline("\n");

					if (path.length==1 && sys.map[path.tiles[0].y*MAP_SIZE+path.tiles[0].x].building.type!=TILE_BUILDING_NONE) {
						entity->tileDest = entity->tilePos;
						entity->nextTileDest = entity->tilePos;
					} else {
						entity->nextTileDest = path.tiles[0];
					}

					if (path.tiles[0].large == entity->tilePos.large) {
						entity->tileDest = entity->tilePos;
					}
				}
			}

			int2_t diff = idiff2(entity->tilePos, entity->job);
			if (diff.x >= -1 && diff.x <= 1 && diff.y >= -1 && diff.y <= 1) {
				sys.map[entity->job.y*MAP_SIZE+entity->job.x].building.health -= 0.01f;
			}

			tile_t* tile = &sys.map[entity->tilePos.y*MAP_SIZE+entity->tilePos.x];
			vec2_t pos = G_TileSpaceToScreenSpace(vec2(
				entity->tilePos.x + ((float)entity->nextTileDest.x-entity->tilePos.x)*entity->aniMove,
				entity->tilePos.y + ((float)entity->nextTileDest.y-entity->tilePos.y)*entity->aniMove
			), tile->height);
			R_BlitBitmapAtlas(sys.mapBitmap, (int)entity->aniFrame*16, 112, 16, 16, sub2(add2(pos, vec2(0, 1.25f)), sys.cameraPos));
		}
	}

	// char* testStr = "Old Tom Bombadil is a merry fellow\nBright blue his jacket is and his boots are yellow\nReeds by the shady pool, lilies on the water\nOld Tom Bombadil and the river-daughter";
	// font_text_t* text = Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, testStr, (font_settings_t){20});
	// R_BlitFontBitmaps(sys.fontBitmap, text, vec2(-19.0f, 10.0f));

	str_set_allocator(&sys.scratchBuffer);

	R_BlitFontBitmaps(
		sys.fontBitmap, 
		Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("Wood: %i", sys.wood), (font_settings_t){100}),
		vec2(-19.0f, 11.0f)
	);

	// R_BlitFontBitmaps(
	// 	sys.fontBitmap, 
	// 	Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("mouse pos %f,%f", mouseInWorldSpace.x, mouseInWorldSpace.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 11.0f)
	// );
	// R_BlitFontBitmaps(
	// 	sys.fontBitmap, 
	// 	Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("mouse tile %f, %f", mouseTile.x, mouseTile.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 10.0f)
	// );
	// R_BlitFontBitmaps(
	// 	sys.fontBitmap, 
	// 	Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("w down %u", video.keyboard[KEY_W].down), (font_settings_t){100}),
	// 	vec2(-19.0f, 9.0f)
	// );
	// R_BlitFontBitmaps(
	// 	sys.fontBitmap, 
	// 	Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("tile0 height %i", sys.map[0].height), (font_settings_t){100}),
	// 	vec2(-19.0f, 8.0f)
	// );
	// vec2_t pos = G_TileSpaceToScreenSpace(vec2(0, 0), /*tile->height*/0);
	// R_BlitFontBitmaps(
	// 	sys.fontBitmap, 
	// 	Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("tile0 pos %f,%f", pos.x, pos.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 7.0f)
	// );

	// if (sys.selectedWorker) {
	// 	R_BlitFontBitmaps(
	// 		sys.fontBitmap, 
	// 		Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, str_format("worker %i,%i", sys.selectedWorker->tilePos.x, sys.selectedWorker->tilePos.y), (font_settings_t){100}),
	// 		vec2(-19.0f, 6.0f)
	// 	);
	// }
	
	// clear_allocator(&sys.scratchBuffer);
	sys.scratchBuffer.stackptr = 0;
	zero_memory(sys.scratchBuffer.address, sys.scratchBuffer.size);
	Sys_OutputFrameAndSync();
}