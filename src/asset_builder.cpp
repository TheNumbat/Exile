
#include "basic_types.h"

#include "str/platform_strings.h"
#include "str/platform_strings.cpp"

#include "platform_api.h"
#include "platform.h"

#define BUILDER
#include "asset.h"

#include <iostream>
#include <cstring>
using std::cout;
using std::endl;

u32 num_strings = 0;

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

		// just leak this
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
		cout << "Failed to open file" << argv[1] << endl;
		return 1;
	}

	err = platform_create_file(&assets_out, string_from_c_str(argv[2]), open_file_create);

	if(!err.good) {
		cout << "Failed to create file" << argv[2] << endl;
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
		file_asset asset;

		if(strcmp(def_strings[i].c_str, "bitmap") == 0) {
			if(def_strings[i + 1].len > 128) {
				cout << "Name too long! Max 128" << endl;
				return 1;
			}

			memcpy(asset.name, def_strings[i + 1].c_str, def_strings[i + 1].len);
			asset.type = asset_bitmap;

			// load bitmap

			// calculate total asset size for next asset member

			// write asset

		} else {
			cout << "Only bitmaps for now!" << endl;
			return 1;
		}
	}

	platform_close_file(&assets_out);
	cout << "Done!" << endl;

	return 0;
}
