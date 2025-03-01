//
//  Created by Matt Hartley on 28/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include "bitmap.h"

#include "font.h"


font_text_t* Fnt_Text(allocator_t* allocator, embedded_font_t* font, char* str, font_settings_t settings) {
    v2 char_pos = {0};

	int numChars = 0;
	for (char* s=str; *s; ++s) {
		if (*s!='\n' && *s!='\t' && *s!=' ') {
			++numChars;
		}
	}

	font_text_t* result = push_memory(allocator, sizeof(font_text_t)+sizeof(font_text_char_t)*numChars);
	int resultIndex = 0;



	for (int i=0; *str; ++i,++str) {
		if(*str == '\n') {
newLine:
			char_pos = vec2(0.0f, char_pos.y - 1.5f); // line spacing
			continue;
		}
		
		
		font_kern_result_t kern = font_get_kerning(font, str[0], str[1]);

		if(*str != ' ' && *str != '\t') {
			vec2_t pos = {char_pos.x, char_pos.y - kern.y*0.125f};
			result->chars[resultIndex++] = (font_text_char_t){*str, pos};
		}

		char_pos.x += (8.0f-kern.x) * 0.125f;

		if (settings.wrap_width>1.0f && *str == ' ') {
			char* scan = str+1;
			v2 scan_pos = char_pos;
			while (*scan && *scan != '\n' && *scan != ' ') {
				u8 xkern = font->kerning.offsets[*scan] >> 4;
				scan_pos.x += (8.0f-xkern) * 0.125f;
				if (scan_pos.x > settings.wrap_width) {
					// *str-- = '\n';
					// break;
					goto newLine;
				}
				++scan;
			}
		}
    }

	result->numChars = numChars;
    return result;
}

bitmap_t* Fnt_GenBitmap(allocator_t* allocator, embedded_font_t* font) {
    bitmap_t* bitmap = alloc_memory(allocator, sizeof(bitmap_t)+128*64*sizeof(u32));
    zero_memory(bitmap->data, 128*64*sizeof(u32));
    FOR (c, 128) {
        int charx = c%16 * 8;
		int chary = c/(16) * 8;
        FOR (p, 64) {
            int x = charx + (p%8);
            int y = chary + (p/8);
            if (font->data[c] & ((u64)0x1 << p)) {
                bitmap->data[y * 128 + x] = 0xFFFFFFFF;
            }
        }
    }

    bitmap->width = 128;
    bitmap->height = 64;
    bitmap->size = bitmap->width + bitmap->height;
    // gfx_texture_t texture = gfx_create_texture(bitmap);
    // free_memory_in(allocator, bitmap);
    return bitmap;
}
