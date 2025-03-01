//
//  Created by Matt Hartley on 24/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <core/core.h>
#include <core/math.h>
#include "system.h"
#include "bitmap.h"
#include "render.h"

extern video_t video;

int2_t R_ConvertPointToScreenSpace(vec2_t pos) {
	// int2_t screen = {
	// 	(pos.x-video.worldSpaceMin.x) / (video.worldSpaceMax.x-video.worldSpaceMin.x) * (float)video.framebufferSize.x,
	// 	(pos.y-video.worldSpaceMin.y) / (video.worldSpaceMax.y-video.worldSpaceMin.y) * (float)video.framebufferSize.y,
	// };
	int2_t screen = {
		roundf((pos.x/(video.worldSpace.x) + 0.5f) * (float)video.framebufferSize.x),
		roundf((pos.y/(video.worldSpace.y) + 0.5f) * (float)video.framebufferSize.y),
	};
	return screen;
}

void R_ConvertPointsToScreenSpace(int2_t* dest, vec2_t* positions, int num) {
	FOR (i, num) {
		dest[i] = R_ConvertPointToScreenSpace(positions[i]);
	}
}

R_FUNC void R_DrawFbmBackground() {
	u32* fb = video.framebuffer;
	int2_t framebufferSize = video.framebufferSize;
	static int index = 0;
	FOR (i, framebufferSize.x*framebufferSize.y) {
		float x = (float)(i%framebufferSize.x + index) * 0.1f;
		float y = (float)((int)(i+(index*framebufferSize.x))/framebufferSize.x) * 0.1f;
		u8 c = fbm(vec2(x, y)) * 255.0f;
		x *= 4.0f;
		y *= 4.0f;
		u8 c2 = fbm(vec2(x, y)) * 255.0f;
		x *= 4.0f;
		y *= 4.0f;
		u8 c3 = fbm(vec2(x, y)) * 255.0f;
		c = c/3 + c2/3 + c3/3;
		fb[i] = 255<<24 | 0<<16 | (c)<<8 | (0)<<0;
	}
	++index;
	index %= (framebufferSize.x*framebufferSize.y);
}

R_FUNC void R_DrawNoiseBackground() {
	u32* fb = video.framebuffer;
	int2_t framebufferSize = video.framebufferSize;
	static float index = 0.0f;
	FOR (y, framebufferSize.y)
	FOR (x, framebufferSize.x) {
		float xf = (int)((float)x * 1 + index);
		float yf = (int)((float)y * 1 + index);
		u8 c = rand2d(vec2(xf, yf)) * 255.0f;
		// x *= 4.0f;
		// y *= 4.0f;
		// u8 c2 = fbm(vec2(x, y)) * 255.0f;
		// x *= 4.0f;
		// y *= 4.0f;
		// u8 c3 = fbm(vec2(x, y)) * 255.0f;
		// c = c/3 + c2/3 + c3/3;
		fb[y*framebufferSize.x+x] = 255<<24 | 0<<16 | (c)<<8 | (0)<<0;
	}
	index += 1;
	// index %= (framebufferSize.x*framebufferSize.y);
}

R_FUNC void R_DrawQuad(vec2_t pos, vec2_t size, u32 color) {
	u32* fb = video.framebuffer;

	vec2_t vertices[] = {
		pos,
		add2(pos, vec2(size.x, 0)),
		add2(pos, size),
		add2(pos, vec2(0, size.y)),
	};

	// int2_t screenPos = {
	// 	(pos.x-worldSpaceMin.x) / (worldSpaceMax.x-worldSpaceMin.x) * (float)video.framebufferSize.x,
	// 	(pos.y-worldSpaceMin.y) / (worldSpaceMax.y-worldSpaceMin.y) * (float)video.framebufferSize.y,
	// };
	// int2_t screenSize = {
	// 	(size.x) / (worldSpaceMax.x-worldSpaceMin.x) * (float)video.framebufferSize.x,
	// 	(size.y) / (worldSpaceMax.y-worldSpaceMin.y) * (float)video.framebufferSize.y,
	// };

	int2_t screenPos[4];
	R_ConvertPointsToScreenSpace(screenPos, vertices, 4);

	// clip
	if (screenPos[0].x >= video.framebufferSize.x ||
		screenPos[1].x < 0 ||
		screenPos[0].y >= video.framebufferSize.y ||
		screenPos[2].y < 0) {
		// cull
		return;
	}
	if (screenPos[0].x < 0) {
		screenPos[0].x = 0;
		screenPos[3].x = 0;
	}
	if (screenPos[0].y < 0) {
		screenPos[0].y = 0;
		screenPos[1].y = 0;
	}
	if (screenPos[1].x >= video.framebufferSize.x) {
		screenPos[1].x = video.framebufferSize.x;
		screenPos[2].x = video.framebufferSize.x;
	}
	if (screenPos[2].y >= video.framebufferSize.y) {
		screenPos[2].y = video.framebufferSize.y;
		screenPos[3].y = video.framebufferSize.y;
	}

	// int2_t i2 = {}
	// FOR (y, screenSize.y)
	// FOR (x, screenSize.x) {
	// 	int index = (screenPos.y+y)*video.framebufferSize.x + (screenPos.x+x);
	// 	fb[index] = color;
	// }
	for (int y=screenPos[0].y; y<screenPos[2].y; ++y)
	for (int x=screenPos[0].x; x<screenPos[1].x; ++x) {
		fb[y*video.framebufferSize.x+x] = color;
	}
}

R_FUNC void R_DrawQuadOutline(vec2_t pos, vec2_t size, u32 color) {
	u32* fb = video.framebuffer;

	int2_t screenPos = {
		(pos.x-video.worldSpaceMin.x) / (video.worldSpaceMax.x-video.worldSpaceMin.x) * (float)video.framebufferSize.x,
		(pos.y-video.worldSpaceMin.y) / (video.worldSpaceMax.y-video.worldSpaceMin.y) * (float)video.framebufferSize.y,
	};
	int2_t screenSize = {
		(size.x) / (video.worldSpaceMax.x-video.worldSpaceMin.x) * (float)video.framebufferSize.x,
		(size.y) / (video.worldSpaceMax.y-video.worldSpaceMin.y) * (float)video.framebufferSize.y,
	};

	// clip
	if (screenPos.x >= video.framebufferSize.x ||
		screenPos.x+screenSize.x < 0 ||
		screenPos.y >= video.framebufferSize.y ||
		screenPos.y+screenSize.y < 0) {
		// cull
		return;
	}
	if (screenPos.x < 0) {
		screenSize.x -= screenPos.x;
		screenPos.x = 0;
	}
	if (screenPos.y < 0) {
		screenSize.y -= screenPos.y;
		screenPos.y = 0;
	}
	if ((screenPos.x+screenSize.x) >= video.framebufferSize.x) {
		screenSize.x = video.framebufferSize.x - screenPos.x;
	}
	if ((screenPos.y+screenSize.y) >= video.framebufferSize.y) {
		screenSize.y = video.framebufferSize.y - screenPos.y;
	}

	for (int y=screenPos.y; y<screenPos.y+screenSize.y; ++y) {
		fb[y*video.framebufferSize.x + (screenPos.x + 0)] = color;
		fb[y*video.framebufferSize.x + (screenPos.x + screenSize.x - 1)] = color;
	}
	for (int x=screenPos.x; x<screenPos.x+screenSize.x; ++x) {
		fb[(screenPos.y + 0)*video.framebufferSize.x + x] = color;
		fb[(screenPos.y + screenSize.y - 1)*video.framebufferSize.x + x] = color;
	}
	// FOR (x, screenSize.x) {
	// 	int index = (screenPos.y+y)*video.framebufferSize.x + (screenPos.x+x);
	// 	fb[index] = 0xFFFF0000;
	// }

	// FOR (y, screenSize.y)
	// FOR (x, screenSize.x) {
	// 	int index = (screenPos.y+y)*video.framebufferSize.x + (screenPos.x+x);
	// 	fb[index] = color;
	// }
}

R_FUNC void R_BlitBitmap(bitmap_t* bitmap, vec2_t pos) {
	int2_t screenPos = R_ConvertPointToScreenSpace(pos);
	u32* fb = video.framebuffer;
	FOR (y, bitmap->height)
	FOR (x, bitmap->width) {
		int fbIndex = (screenPos.y+y)*video.framebufferSize.x + (screenPos.x+x);
		u32 texel = bitmap->data[y*bitmap->width+x];
		if (texel) {
			fb[fbIndex] = texel;
		}
	}
}

R_FUNC b32 R_Clip(int2_t* pos, int2_t* size) {
	if (pos->x >= video.framebufferSize.x ||
		pos->x+size->x < 0 ||
		pos->y >= video.framebufferSize.y ||
		pos->y+size->y < 0) {
		// cull
		return TRUE;
	}
	if (pos->x < 0) {
		size->x -= pos->x;
		pos->x = 0;
	}
	if (pos->y < 0) {
		size->y -= pos->y;
		pos->y = 0;
	}
	if ((pos->x+size->x) >= video.framebufferSize.x) {
		size->x = video.framebufferSize.x - pos->x;
	}
	if ((pos->y+size->y) >= video.framebufferSize.y) {
		size->y = video.framebufferSize.y - pos->y;
	}

	return FALSE;
}

R_FUNC void R_BlitBitmapAtlas(bitmap_t* bitmap, int atlasX, int atlasY, int width, int height, vec2_t pos) {
	int2_t screenPos = R_ConvertPointToScreenSpace(pos);
	int2_t size = {width, height};
	if (R_Clip(&screenPos, &size)) {
		return;
	}
	u32* fb = video.framebuffer;
	FOR (y, size.y)
	FOR (x, size.x) {
		int fbIndex = (screenPos.y+y)*video.framebufferSize.x + (screenPos.x+x);
		u32 texel = bitmap->data[(atlasY+y)*bitmap->width+(atlasX+x)];
		if (texel) {
			fb[fbIndex] = texel;
		}
	}
}
