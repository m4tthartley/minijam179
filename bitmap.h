//
//  Created by Matt Hartley on 26/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <core/core.h>

#pragma pack(push, 1)
typedef struct {
	char header[2];
	u32 size;
	u16 reserved1;
	u16 reserved2;
	u32 offset;
	
	// Windows BITMAPINFOHEADER
	u32 headerSize;
	i32 bitmapWidth;
	i32 bitmapHeight;
	u16 colorPlanes;
	u16 colorDepth;
	u32 compression;
	u32 imageSize;
	i32 hres;
	i32 vres;
	u32 paletteSize;
	u32 importantColors;
} bmp_header_t;
#pragma pack(pop)

typedef struct {
	u32 size;
	u32 width;
	u32 height;
	u32 data[];
} bitmap_t;

bitmap_t* LoadBitmap(allocator_t* allocator, char* filename);

#endif
