//
//  Created by Matt Hartley on 23/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <core/core.h>
#include <core/core.c>
#include "core/platform.h"
#include "game.h"
#include "render.h"
#include "bitmap.h"
#include "font.c"
#include "system.h"

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

	sys.testBitmap = LoadBitmap(&sys.assetMemory, "assets/test.bmp");
	sys.mapBitmap = LoadBitmap(&sys.assetMemory, "assets/map.bmp");

	sys.fontBitmap = Fnt_GenBitmap(&sys.assetMemory, &FONT_DEFAULT);

	// sys.pianoTest = Sys_LoadWave(&sys.assetMemory, ReadEntireFile(&sys.assetMemory, "/Users/matt/Desktop/piano.wav"));
	// Sys_QueueSound(sys.pianoTest, 0.5f);

	FOR (y, MAP_SIZE)
	FOR (x, MAP_SIZE) {
		float height = fbm(vec2((float)x*0.1f, (float)y*0.1f)) * 10.0f;
		sys.map[y*MAP_SIZE+x].height = height;
	}
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

void G_Update() {
	Sys_PollEvents();
	vec2_t cameraSpeed = {0};
	cameraSpeed.x = (float)video.keyboard[KEY_D].down - (float)video.keyboard[KEY_A].down;
	cameraSpeed.y = (float)video.keyboard[KEY_W].down - (float)video.keyboard[KEY_S].down;
	sys.cameraPos = add2(sys.cameraPos, mul2f(cameraSpeed, 0.1f));
	if (video.keyboard[KEY_S].down) {
		int x = 0;
	}
	
	R_DrawQuad(vec2(-video.worldSpace.x/2, -video.worldSpace.y/2), vec2(video.worldSpace.x, video.worldSpace.y), 0xFF000000);
	// R_DrawNoiseBackground();
	R_DrawQuad(vec2(2, 2), vec2(5.0f, 5.0f), 0xFFFF00FF);
	// R_DrawQuadOutline(vec2(-5, -2), vec2(3.0f, 5.0f), 0xFF0000FF);

	// R_BlitBitmap(sys.mapBitmap, vec2(0, 0));
	for (int y=MAP_SIZE-1; y>-1; --y)
	FOR (x, MAP_SIZE) {
		tile_t* tile = sys.map + (y*MAP_SIZE+x);
		R_BlitBitmapAtlas(sys.mapBitmap, 0, 0, 16, 16, sub2(vec2(((float)x*1.0f)+((float)y*1.0f), ((float)y*0.5f)-((float)x*0.5f)+((float)tile->height*0.5f)), sys.cameraPos));
	}

	char* testStr = "Old Tom Bombadil is a merry fellow\nBright blue his jacket is and his boots are yellow\nReeds by the shady pool, lilies on the water\nOld Tom Bombadil and the river-daughter";
	font_text_t* text = Fnt_Text(&sys.scratchBuffer, &FONT_DEFAULT, testStr, (font_settings_t){20});
	R_BlitFontBitmaps(sys.fontBitmap, text, vec2(-19.0f, 10.0f));
	
	// clear_allocator(&sys.scratchBuffer);
	sys.scratchBuffer.stackptr = 0;
	zero_memory(sys.scratchBuffer.address, sys.scratchBuffer.size);
	Sys_OutputFrameAndSync();
}