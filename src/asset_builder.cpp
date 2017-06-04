
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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

#pragma pack(push, 1)
struct bitmap_header {
	u16 type; // "BM"
	u32 size;
	u16 zero;
	u16 zero_;
	u32 memoffset;
	u32 second_header_size;
	i32 width;
	i32 height;
	u16 planes;
	u16 bitsperpipxel;
};
#pragma pack(pop)

bool whitespace(char c) {
	return c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

void extract_strings(char* mem, u32 size, string strings[]) {

	u32 strstart = 0;
	u32 cursor = 0;

	while(cursor < size) {

		while(whitespace(mem[cursor])) {
			cursor++;
			if(cursor >= size)
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

	if(num_strings % 3 != 0) {
		cout << "Wrong amount of records " << num_strings << endl;
		return 1;
	}

	asset_file_header header;
	header.num_assets = num_strings / 3;
	platform_write_file(&assets_out, (void*)&header, sizeof(asset_file_header));

	for(u32 i = 0; i < num_strings; i += 3) {
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

			bitmap_header* bmp_header = (bitmap_header*)bmp_mem;
			u8* pixels = (u8*)bmp_mem + bmp_header->memoffset;

			file_asset_bitmap bitmap;
			bitmap.width = bmp_header->width;
			bitmap.height = bmp_header->height;

			// calculate total asset size for next asset member
			u32 pixel_stride =  bmp_header->width * bmp_header->bitsperpipxel / 8;
			u32 pixel_size = (bmp_header->height * pixel_stride);
			asset.next = sizeof(file_asset_header) + sizeof(file_asset_bitmap) + pixel_size;

			// BGRA -> RGBA
			for(u8* current = pixels; current != pixels + pixel_size; current += 4) {
				u8 b = *(current);
				u8 g = *(current + 1);
				u8 r = *(current + 2);
				u8 a = *(current + 3);
				*current 	   = r;
				*(current + 1) = g;
				*(current + 2) = b;
				*(current + 3) = a;
			}

			// write asset
			platform_write_file(&assets_out, (void*)&asset, sizeof(file_asset_header));
			platform_write_file(&assets_out, (void*)&bitmap, sizeof(file_asset_bitmap));
			platform_write_file(&assets_out, (void*)pixels, pixel_size);

			platform_heap_free(bmp_mem);
		} else {
			cout << "Only bitmaps for now!" << endl;
			return 1;
		}
	}

	for(u32 i = 0; i < num_strings; i++)
		free_string(def_strings[i], &platform_heap_free);

	platform_close_file(&assets_out);

	cout << "Done!" << endl;

	return 0;
}
