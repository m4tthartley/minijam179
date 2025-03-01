//
//  Created by Matt Hartley on 26/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <core/core.h>
#include "bitmap.h"

bitmap_t* LoadBitmap(allocator_t* allocator, char* filename) {
	// TODO replace with platform functions
	// FILE* fontFile;
	// void* fontData;
	// bmp_heaer_t* header;
	// u32* palette;
	// char* data;
	// int rowSize;
	
	file_t file = file_open(filename);
	stat_t fileinfo = file_stat(file);
	int fileSize = fileinfo.size;
	void* fileData = alloc_memory(allocator, fileSize);
	file_read(file, 0, fileData, fileSize);
	file_close(file);


	// fontFile = fopen(filename, "r"); // todo: this stuff crashes when file not found
	// if(!fontFile) {
	// 	print_error("Unable to open file: %s", filename);
	// 	return NULL;
	// }
	// fseek(fontFile, 0, SEEK_END);
	// fileSize = ftell(fontFile);
	// fontData = malloc(fileSize);
	// rewind(fontFile);
	// fread(fontData, 1, fileSize, fontFile);
	// fclose(fontFile);
	
	bmp_header_t* header = (bmp_header_t*)fileData;
	u32* palette = (u32*)((char*)fileData+14+header->headerSize);
	u8* data = (u8*)fileData+header->offset;
	int rowSize = ((header->colorDepth*header->bitmapWidth+31) / 32) * 4;

	// Possibly check whether to alloc or push
	bitmap_t* result = alloc_memory(allocator, sizeof(bitmap_t) + sizeof(u32)*header->bitmapWidth*header->bitmapHeight);
	result->size = header->size;
	result->width = header->bitmapWidth;
	result->height = header->bitmapHeight;
	
	u32* image = (u32*)(result + 1);
	// image = (u32*)malloc(sizeof(u32)*header->bitmapWidth*header->bitmapHeight);
	//{for(int w=0; w<header.bitmapHeight}
	{
		// int row;
		// int pixel;
		for(int row=0; row<header->bitmapHeight; ++row) {
			int bitIndex=0;
			//printf("row %i \n", row);
// 			if(row==255) {
// 				DebugBreak();
// 			}
			for(int pixel=0; pixel<header->bitmapWidth; ++pixel) {//while((bitIndex/8) < rowSize) {
				u32* chunk = (u32*)((char*)fileData+header->offset+(row*rowSize)+(bitIndex/8));
				u32 pi = *chunk;
				if(header->colorDepth<8) {
					pi >>= (header->colorDepth-(bitIndex%8));
				}
				pi &= (((i64)1<<header->colorDepth)-1);
				if(header->colorDepth>8) {
					image[row*header->bitmapWidth+pixel] = pi;
				} else {
					image[row*header->bitmapWidth+pixel] = palette[pi];
				}

				image[row*header->bitmapWidth+pixel] |= 0xFF << 24;

				if(image[row*header->bitmapWidth+pixel]==0xFF000000 ||
				   image[row*header->bitmapWidth+pixel]==0xFFFF00FF) {
					image[row*header->bitmapWidth+pixel] = 0;
				}

				bitIndex += header->colorDepth;
			}
		}
	}

	// free(fontData);
	
	// result.data = image;
	// result.header = header;
	return result;
}