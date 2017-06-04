
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4456)
#pragma warning(disable : 4505)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#pragma warning(pop)

#include "basic_types.h"

#include "str/platform_strings.h"
#include "str/platform_strings.cpp"

#include "platform_api.h"
#include "platform.h"
#include "platform_win32.cpp"

#define BUILDER
#include "asset.h"

#include <iostream>
#include <cstring>
using std::cout;
using std::endl;

u32 num_strings = 0;
u32 num_lines = 0;

bool newline(char c) {
	return c == '\n' || c == '\r';
}

void extract_strings(char* mem, u32 size, string strings[]) {

	u32 strstart = 0;
	u32 cursor = 0;

	while(cursor < size) {

		if(mem[cursor] == '\r') {
			num_lines++;
			cursor += 2;
		}
		if(cursor >= size) {
			return;	
		}

		strstart = cursor;

		for(; mem[cursor] != ':' && cursor < size; cursor++);

		u32 length = cursor - strstart + 1;

		strings[num_strings] = make_string(length, &platform_heap_alloc);
		memcpy(strings[num_strings].c_str, mem + strstart, length - 1);
		strings[num_strings].c_str[length] = 0;
		strings[num_strings].len = length;

		num_strings++;

		if(num_strings >= 3072) {
			cout << "Too many records!" << endl;
			exit(1);
		}

		cursor++;
	}	
}

int main(int argc, char** argv) {

	if(argc < 3) {
		cout << "You must pass an input and output file name!" << endl;
		return 1;
	}

	platform_file def_file, assets_out;
	platform_error err = platform_create_file(&def_file, string_from_c_str(argv[1]), open_file_existing);

	if(!err.good) {
		cout << "Failed to open file " << argv[1] << endl;
		return 1;
	}

	err = platform_create_file(&assets_out, string_from_c_str(argv[2]), open_file_create);

	if(!err.good) {
		cout << "Failed to create file " << argv[2] << endl;
		return 1;
	}

	u32 def_size = platform_file_size(&def_file);

	char* def_mem = (char*)platform_heap_alloc(def_size);
	platform_read_file(&def_file, def_mem, def_size);
	platform_close_file(&def_file);

	string def_strings[3072];
	extract_strings(def_mem, def_size, def_strings);
	platform_heap_free(def_mem);

	asset_file_header header;
	header.num_assets = num_lines;
	platform_write_file(&assets_out, (void*)&header, sizeof(asset_file_header));

	for(u32 i = 0; i < num_strings;) {
		file_asset_header asset;

		if(def_strings[i + 1].len > 128) {
			cout << "Name too long! Max 128" << endl;
			return 1;
		}

		memcpy(asset.name, def_strings[i + 1].c_str, def_strings[i + 1].len);

		if(strcmp(def_strings[i].c_str, "bitmap") == 0) {

			asset.type = asset_bitmap;

			// load bitmap
			platform_file bmp_in;
			err = platform_create_file(&bmp_in, def_strings[i + 2], open_file_existing);
			if (!err.good) {
				cout << "Failed to open file " << def_strings[i + 2].c_str << endl;
				return 1;
			}

			u32 bmp_size = platform_file_size(&bmp_in);
			void* bmp_mem = platform_heap_alloc(bmp_size);
			platform_read_file(&bmp_in, bmp_mem, bmp_size);
			platform_close_file(&bmp_in);

			file_asset_bitmap bitmap;
			u8* pixels = stbi_load_from_memory((u8*)bmp_mem, bmp_size, &bitmap.width, &bitmap.height, NULL, 4);

			// calculate total asset size for next asset member
			u32 pixel_stride =  bitmap.width * 4;
			u32 pixel_size = bitmap.height * pixel_stride;
			asset.next = sizeof(file_asset_header) + sizeof(file_asset_bitmap) + pixel_size;

			// write asset
			platform_write_file(&assets_out, (void*)&asset, sizeof(file_asset_header));
			platform_write_file(&assets_out, (void*)&bitmap, sizeof(file_asset_bitmap));
			
			// flip bitmap
			u8* pixel_last = pixels + pixel_size - pixel_stride;
			for(; pixel_last != pixels; pixel_last -= pixel_stride) {
				platform_write_file(&assets_out, (void*)pixel_last, pixel_stride);
			}
			platform_write_file(&assets_out, (void*)pixel_last, pixel_stride);

			platform_heap_free(bmp_mem);

			i += 3;
		
		} else if(strcmp(def_strings[i].c_str, "font") == 0) {

			asset.type = asset_font;

			// load font
			platform_file font_in;
			err = platform_create_file(&font_in, def_strings[i + 5], open_file_existing);
			if (!err.good) {
				cout << "Failed to open file " << def_strings[i + 5].c_str << endl;
				return 1;
			}

			u32 font_size = platform_file_size(&font_in);
			void* font_mem = platform_heap_alloc(font_size);
			platform_read_file(&font_in, font_mem, font_size);
			platform_close_file(&font_in);

			// parse load data
			u32 point_size = atoi(def_strings[i + 2].c_str);
			u32 cp_begin = atoi(def_strings[i + 3].c_str);
			u32 cp_end = atoi(def_strings[i + 4].c_str);
			u32 cp_num = cp_end - cp_begin + 1;
			stbtt_packedchar* packedchars = (stbtt_packedchar*)platform_heap_alloc(cp_num * sizeof(stbtt_packedchar));

			stbtt_fontinfo font_info;
			stbtt_pack_context pack_context;
			i32 ascent, baseline;
			f32 scale;

			// get font data
			stbtt_InitFont(&font_info, (u8*)font_mem, 0);
			scale = stbtt_ScaleForPixelHeight(&font_info, (f32)point_size);
			stbtt_GetFontVMetrics(&font_info, &ascent, 0, 0);
			baseline = (i32) (ascent * scale);

			// TODO(max): is 1024x1024 always good?
			u32 pixel_stride =  1024 * 1;
			u32 pixel_size = 1024 * pixel_stride;
			u8* bake_mem = (u8*)platform_heap_alloc(pixel_size);

			// pack chars to font
			stbtt_PackBegin(&pack_context, bake_mem, 1024, 1024, pixel_stride, 2, NULL);
			// stbtt_PackSetOversampling(&pack_context, 2, 2);
			stbtt_PackFontRange(&pack_context, (u8*)font_mem, 0, (f32)point_size, cp_begin, cp_num, packedchars);
			stbtt_PackEnd(&pack_context);
			// stbtt_BakeFontBitmap((u8*)font_mem, 0, (f32)point_size, bake_mem, 1024, 1024, cp_begin, cp_num, packedchars);

			// write asset
			asset.next = sizeof(file_asset_header) + sizeof(file_asset_font) + cp_num * sizeof(stbtt_packedchar) + pixel_size;
			platform_write_file(&assets_out, (void*)&asset, sizeof(file_asset_header));

			// write font asset
			file_asset_font font;
			font.num_glyphs = cp_num;
			font.baseline = baseline;
			platform_write_file(&assets_out, (void*)&font, sizeof(file_asset_font));

			// write glyph data
			// this assumes file_glyph_data is the same as stbtt_packedchar
			assert(sizeof(stbtt_packedchar) == sizeof(file_glyph_data));
			platform_write_file(&assets_out, (void*)packedchars, cp_num * sizeof(stbtt_packedchar));

			// write texture
			// platform_write_file(&assets_out, (void*)bake_mem, pixel_size);
			u8* pixel_last = bake_mem + pixel_size - pixel_stride;
			for(; pixel_last != bake_mem; pixel_last -= pixel_stride) {
				platform_write_file(&assets_out, (void*)pixel_last, pixel_stride);
			}
			platform_write_file(&assets_out, (void*)pixel_last, pixel_stride);

			platform_heap_free(bake_mem);
			platform_heap_free(packedchars);
			platform_heap_free(font_mem);

			i += 6;

		} else {

			cout << "Only images/fonts for now!" << endl;
			return 1;
		}
	}

	for(u32 i = 0; i < num_strings; i++)
		free_string(def_strings[i], &platform_heap_free);

	platform_close_file(&assets_out);

	cout << "Done!" << endl;

	return 0;
}
