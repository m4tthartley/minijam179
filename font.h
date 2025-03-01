//
//  Created by Matt Hartley on 28/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <core/core.h>
#include <core/math.h>
#include <core/font.h>

#include "bitmap.h"

typedef struct {
	float wrap_width;
} font_settings_t;

typedef struct {
	u8 index;
	vec2_t pos;
} font_text_char_t;

typedef struct {
	int numChars;
	font_text_char_t chars[];
} font_text_t;

font_text_t* Fnt_Text(allocator_t* allocator, embedded_font_t* font, char* str, font_settings_t settings);
bitmap_t* Fnt_GenBitmap(allocator_t* allocator, embedded_font_t* font);
