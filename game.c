//
//  Created by Matt Hartley on 23/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <objc/runtime.h>

#include "game.h"
#include "system.h"

#include "render.c"
#include "bitmap.c"
#include "font.c"
#include "system_resource.c"

#define CORE_IMPL
#include <core/sysaudio.h>
#include <core/sys.h>
#include <core/core.h>
#include <core/math.h>
#include <core/font.h>
#include <core/wave.h>
#include <core/hotreload.h>

sysaudio_t audio;
video_t video;
gamestate_t game;

void ObjcDebug() {
	int numClasses = objc_getClassList(NULL, 0);
	Class* classes = sys_alloc_memory(sizeof(Class)*numClasses);
	objc_getClassList(classes, numClasses);
	FOR (i, numClasses) {
		print("objc class: %s", class_getName(classes[i]));
	}
}

file_data_t* ReadEntireFile(allocator_t* allocator, char* filename) {
	file_data_t* result = NULL;

	file_t file = sys_open(filename);
	if (file) {
		stat_t info = sys_stat(file);
		result = alloc_memory(allocator, sizeof(file_data_t)+info.size);
		sys_copy_memory(result, &info, sizeof(info));
		size_t readResult = sys_read(file, 0, result+1, info.size);
		if (readResult != info.size) {
			return NULL;
		}
		sys_close(file);
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
	point = add2(point, game.cameraPos);
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
	game.map[pos.y*MAP_SIZE+pos.x].building.type = TILE_BUILDING_TOWER;
	game.map[pos.y*MAP_SIZE+pos.x].building.varient = randr(0, 3);
	game.map[pos.y*MAP_SIZE+pos.x].building.health = 1.0f;

	if (energy > 0.0f) {
		G_GenTower(int2(pos.x+1, pos.y), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x-1, pos.y), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x, pos.y+1), energy - (0.5f + randf()*0.5f));
		G_GenTower(int2(pos.x, pos.y-1), energy - (0.5f + randf()*0.5f));
	}
}

audio_buffer_t* LoadAudioTrack(char* filename) {
	if (game.audioTrackCount >= array_size(game.audioTracks)) {
		print_error("Audio track array full");
		return NULL;
	}

	// char path[MAX_PATH_LENGTH];
	char* path = Sys_GetResourcePath(&game.scratchBuffer, filename);

	file_data_t* file = ReadEntireFile(&game.assetMemory, path);
	if (!file) {
		print_error("Failed to load file: %s", path);
		return NULL;
	}
	audio_buffer_t* buffer = sys_decode_wave(&game.assetMemory, file);
	char_copy(game.audioTracks[game.audioTrackCount].name, filename, MAX_PATH_LENGTH);
	game.audioTracks[game.audioTrackCount++].buffer = buffer;
	return buffer;
}

void G_Init(sys_window_t* window) {
	// if (!sys_message_box("Green Energy", "Are you sure you want to play?", "Play", "Quit")) {
	// 	exit(0);
	// }

	// window = &program->window;
	// audio = &program->audio;
	// video = &program->video;
	// game = &program->game;

	game.running = TRUE;

	// sys_init_audio(audio, SYSAUDIO_DEFAULT_SPEC);

	float aspect = (float)video.framebufferSize.x / (float)video.framebufferSize.y;
	video.worldSpaceMin = (vec2_t){-10.0f * aspect, -10.0f};
	video.worldSpaceMax = (vec2_t){10.0f * aspect, 10.0f};
	video.worldSpace = (vec2_t){(float)video.framebufferSize.x / 8.0f, (float)video.framebufferSize.y / 8.0f};

	game.assetMemory = virtual_heap_allocator(MB(1500), NULL);
	game.scratchBuffer = virtual_bump_allocator(MB(1), NULL);

	game.testBitmap = LoadBitmap(&game.assetMemory, Sys_GetResourcePath(NULL, "assets/test.bmp"));
	game.mapBitmap = LoadBitmap(&game.assetMemory, Sys_GetResourcePath(NULL, "assets/map.bmp"));

	game.fontBitmap = Fnt_GenBitmap(&game.assetMemory, &FONT_DEFAULT);

	// game.pianoTest = sys_decode_wave(&game.assetMemory, ReadEntireFile(&game.assetMemory, "../resources/piano.wav"));
	// sys_play_sound(&audio, game.pianoTest, 0.5f);

	LoadAudioTrack("sine.wav");

	LoadAudioTrack("dunka_16bit_44k.wav");
	LoadAudioTrack("dunka_8bit_44k.wav");
	LoadAudioTrack("dunka_32bit_44k.wav");
	LoadAudioTrack("dunka_24bit_44k.wav");
	LoadAudioTrack("dunka_32float_44k.wav");
	LoadAudioTrack("dunka_64float_44k.wav");
	
	LoadAudioTrack("dunka_16bit_176k.wav");
	LoadAudioTrack("dunka_16bit_44k_dolby5.1.wav");
	LoadAudioTrack("dunka_16bit_48k.wav");

	LoadAudioTrack("5.1_test_new.wav");
	LoadAudioTrack("7.1_test_new.wav");


	uint8_t output[16] = {0};
	uint8_t buffer[] = {
		0, 0, 150, /**/ 0, 0, 152,
		0, 0, 127, /**/ 0, 0, 126,
		0, 0, 127, /**/ 0, 0, 126,
		0, 0, 127, /**/ 0, 0, 126,
	};
	audio_buffer_t* ab = alloc_memory(&game.assetMemory, sizeof(audio_buffer_t) + sizeof(buffer));
	sys_copy_memory(ab+1, buffer, sizeof(buffer));
	*ab = (audio_buffer_t){
		.sampleCount=4, .sampleRate=44100, .sampleSize=3, .channels=2,
	};
	_mix_sample(&audio, output, &(audio_sound_t){
		.buffer = ab,
		.cursor = 0.0f,
		.volume = 0.5f,
	});
	_mix_sample(&audio, output+8, &(audio_sound_t){
		.buffer = ab,
		.cursor = 1.0f,
		.volume = 0.5f,
	});
	
	LoadAudioTrack("piano.wav");
	LoadAudioTrack("organ.wav");
	LoadAudioTrack("StarWars60.wav");
	LoadAudioTrack("ImperialMarch60.wav");
	LoadAudioTrack("PinkPanther60.wav");

	FOR (y, MAP_SIZE)
	FOR (x, MAP_SIZE) {
		float height = fbm(vec2((float)x*0.05f + 1.0f, (float)y*0.05f + 1.0f)) * 40.0f;
		game.map[y*MAP_SIZE+x].pos = int2(x, y);
		game.map[y*MAP_SIZE+x].height = height;

		if (rand2d(vec2(x, y)) > 0.9f) {
			game.map[y*MAP_SIZE+x].building.type = TILE_BUILDING_TREE;
			game.map[y*MAP_SIZE+x].building.health = 1.0f;
		}

		// float buildingChance = fbm(vec2((float)x*0.2f + 123456.0f, (float)y*0.2f + 123456.0f));
		// if (buildingChance > 0.7f) {
		// 	game.map[y*MAP_SIZE+x].buildingType = TILE_BUILDING_TOWER;
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
	return game.map + (pos.y*MAP_SIZE + pos.x);
}

void G_AddEntity(entity_type_t type, int2_t pos) {
	FOR (i, array_size(game.entities)) {
		if (game.entities[i].type == ENTITY_NONE) {
			game.entities[i] = (entity_t){
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
		.tiles = push_memory(&game.scratchBuffer, sizeof(int2_t)*node.depth),
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
	sys_zero_memory(pathNodes, sizeof(pathNodes));
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
				game.map[tile.y*MAP_SIZE+tile.x].building.type != TILE_BUILDING_NONE) {
				return G_GetNextTileFromPath(pos, node);
			}

			if (tile.x >= 0 &&
				tile.x < MAP_SIZE &&
				tile.y >= 0 &&
				tile.y < MAP_SIZE &&
				game.map[tile.y*MAP_SIZE+tile.x].building.type == TILE_BUILDING_NONE) {
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
	int2_t* pathMem = push_memory(&game.scratchBuffer, sizeof(pos));
	sys_copy_memory(pathMem, &pos, sizeof(pos));
	return (path_t){
		.tiles = pathMem,
		.length = 1,
	};
}

_Bool classesPrinted = FALSE;
void G_Update(sys_window_t* window) {
	sys_button_t* keyboard = window->keyboard;

	sys_set_audio_callback(&audio, sysaudio_default_mixer);
	if (window->keyboard[KEY_1].released) {
		sys_play_sound(&audio, game.pianoTest, 0.5f);
	}
	if (window->keyboard[KEY_2].released) {
		sys_play_sound(&audio, game.dunka1, 0.5f);
	}

	if (keyboard[KEY_DOWN].released) {
		if (game.selectedTrack < game.audioTrackCount-1) {
			++game.selectedTrack;
		}
	}
	if (keyboard[KEY_UP].released) {
		if (game.selectedTrack > 0) {
			--game.selectedTrack;
		}
	}
	if (keyboard[KEY_P].released) {
		if (audio.music.buffer) {
			audio.musicFade = _True;
		} else {
			sys_play_music(&audio, game.audioTracks[game.selectedTrack].buffer, 0.5f);
		}
	}

	vec2_t cameraSpeed = {0};
	cameraSpeed.x = (float)window->keyboard[KEY_D].down - (float)window->keyboard[KEY_A].down;
	cameraSpeed.y = (float)window->keyboard[KEY_W].down - (float)window->keyboard[KEY_S].down;
	game.cameraPos = add2(game.cameraPos, mul2f(cameraSpeed, 0.25f));
	if (window->keyboard[KEY_S].down) {
		int x = 0;
	}

	tile_t* highlightedTile = NULL;

	vec2_t mouseInWorldSpace = vec2(
		((float)window->mouse.pos.x/video.screenSize.x - 0.5f) * video.worldSpace.x,
		((float)window->mouse.pos.y/video.screenSize.y - 0.5f) * video.worldSpace.y
	);
	mouseInWorldSpace = sub2(mouseInWorldSpace, vec2(0.0f, 1.5f));
	vec2_t mouseTile = G_ScreenSpaceToTileSpace(sub2(mouseInWorldSpace, game.cameraPos));
	
	R_DrawQuad(vec2(-video.worldSpace.x/2, -video.worldSpace.y/2), vec2(video.worldSpace.x, video.worldSpace.y), 0xFF000000);

	// R_BlitBitmap(game.mapBitmap, vec2(0, 0));
	for (int y=MAP_SIZE-1; y>-1; --y)
	FOR (x, MAP_SIZE) {
		tile_t* tile = game.map + (y*MAP_SIZE+x);
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
				game.wood += 5;
			}
		}
		
		vec2_t pos = G_TileSpaceToScreenSpace(vec2(x, y), tile->height);
		R_BlitBitmapAtlas(game.mapBitmap, 0, 0, 16, 16, sub2(pos, game.cameraPos));
		// if (highlighted) {
		// 	R_BlitBitmapAtlas(game.mapBitmap, 16, 0, 16, 16, sub2(pos, game.cameraPos));
		// }

		if (tile->building.type == TILE_BUILDING_TREE) {
			R_BlitBitmapAtlas(game.mapBitmap, 48+32, 0, 16, 32, sub2(pos, game.cameraPos));
		}
		if (tile->building.type == TILE_BUILDING_TOWER) {
			// R_BlitBitmapAtlas(game.mapBitmap, 48+32, 0, 16, 32, sub2(pos, game.cameraPos));
			R_BlitBitmapAtlas(game.mapBitmap, 80+(16*tile->building.varient), 32, 16, 48, sub2(pos, game.cameraPos));
		}
	}

	// Highlight tile
	if (highlightedTile) {
		if (window->keyboard[KEY_E].released) {
			G_AddEntity(ENTITY_WORKER, highlightedTile->pos);
		}

		if (window->mouse.left.pressed) {
			FOR (i, array_size(game.entities)) {
				if (game.entities[i].tilePos.large == highlightedTile->pos.large) {
					game.workerDragMode = TRUE;
					game.selectedWorker = game.entities + i;
					break;
				}
			}
		}
		if (window->mouse.left.released) {
			if (game.workerDragMode && game.selectedWorker) {
				game.selectedWorker->tileDest = highlightedTile->pos;
				game.selectedWorker->job = highlightedTile->pos;
				game.workerDragMode = FALSE;
			}
		}

		if (game.workerDragMode && game.selectedWorker) {
			path_t path = G_PathFind(game.selectedWorker->tilePos, highlightedTile->pos);
			FOR (i, path.length) {
				tile_t* tile = &game.map[path.tiles[i].y*MAP_SIZE+path.tiles[i].x];
				vec2_t pos = G_TileSpaceToScreenSpace(vec2(tile->pos.x, tile->pos.y), tile->height);
				R_BlitBitmapAtlas(game.mapBitmap, 16, 0, 16, 16, sub2(pos, game.cameraPos));
			}
		} else {
			vec2_t pos = G_TileSpaceToScreenSpace(vec2(highlightedTile->pos.x, highlightedTile->pos.y), highlightedTile->height);
			R_BlitBitmapAtlas(game.mapBitmap, 16, 0, 16, 16, sub2(pos, game.cameraPos));
		}
	}

	// if (!video.mouse.left.down) {
	// 	game.workerDragMode = FALSE;
	// }

	// Entities
	FOR (i, array_size(game.entities)) {
		entity_t* entity = game.entities + i;
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

					if (path.length==1 && game.map[path.tiles[0].y*MAP_SIZE+path.tiles[0].x].building.type!=TILE_BUILDING_NONE) {
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
				game.map[entity->job.y*MAP_SIZE+entity->job.x].building.health -= 0.01f;
			}

			tile_t* tile = &game.map[entity->tilePos.y*MAP_SIZE+entity->tilePos.x];
			vec2_t pos = G_TileSpaceToScreenSpace(vec2(
				entity->tilePos.x + ((float)entity->nextTileDest.x-entity->tilePos.x)*entity->aniMove,
				entity->tilePos.y + ((float)entity->nextTileDest.y-entity->tilePos.y)*entity->aniMove
			), tile->height);
			R_BlitBitmapAtlas(game.mapBitmap, (int)entity->aniFrame*16, 112, 16, 16, sub2(add2(pos, vec2(0, 1.25f)), game.cameraPos));
		}
	}

	// char* testStr = "Old Tom Bombadil is a merry fellow\nBright blue his jacket is and his boots are yellow\nReeds by the shady pool, lilies on the water\nOld Tom Bombadil and the river-daughter";
	// font_text_t* text = Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, testStr, (font_settings_t){20});
	// R_BlitFontBitmaps(game.fontBitmap, text, vec2(-19.0f, 10.0f));

	str_set_allocator(&game.scratchBuffer);

	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("Wood: %i", game.wood), (font_settings_t){100}),
	// 	vec2(-19.0f, 11.0f)
	// );

	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("mouse pos: %i, %i", window->mouse.pos.x, window->mouse.pos.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 10.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("mouse dt: %i, %i", window->mouse.pos_dt.x, window->mouse.pos_dt.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 9.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("mouse wheel: %i", window->mouse.wheel_dt), (font_settings_t){100}),
	// 	vec2(-19.0f, 8.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("abc mouse buttons: %i, %i", (int)window->mouse.left.down, (int)window->mouse.right.down), (font_settings_t){100}),
	// 	vec2(-19.0f, 7.0f)
	// );

	R_BlitFontBitmaps(
		game.fontBitmap, 
		Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("track %i: %s", game.selectedTrack, game.audioTracks[game.selectedTrack].name), (font_settings_t){100}),
		vec2(-19.0f, 11.0f)
	);

	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("mouse pos %f,%f", mouseInWorldSpace.x, mouseInWorldSpace.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 11.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("mouse tile %f, %f", mouseTile.x, mouseTile.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 10.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("w down %u", video.keyboard[KEY_W].down), (font_settings_t){100}),
	// 	vec2(-19.0f, 9.0f)
	// );
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("tile0 height %i", game.map[0].height), (font_settings_t){100}),
	// 	vec2(-19.0f, 8.0f)
	// );
	// vec2_t pos = G_TileSpaceToScreenSpace(vec2(0, 0), /*tile->height*/0);
	// R_BlitFontBitmaps(
	// 	game.fontBitmap, 
	// 	Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("tile0 pos %f,%f", pos.x, pos.y), (font_settings_t){100}),
	// 	vec2(-19.0f, 7.0f)
	// );

	// if (game.selectedWorker) {
	// 	R_BlitFontBitmaps(
	// 		game.fontBitmap, 
	// 		Fnt_Text(&game.scratchBuffer, &FONT_DEFAULT, str_format("worker %i,%i", game.selectedWorker->tilePos.x, game.selectedWorker->tilePos.y), (font_settings_t){100}),
	// 		vec2(-19.0f, 6.0f)
	// 	);
	// }
	
	// clear_allocator(&game.scratchBuffer);
	game.scratchBuffer.stackptr = 0;
	sys_zero_memory(game.scratchBuffer.address, game.scratchBuffer.size);
}