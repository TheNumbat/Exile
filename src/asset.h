
#pragma once

enum asset_type : u8 {
	asset_bitmap,
	asset_font, 	// TODO(max)
};

struct _asset_bitmap {
	i32 width = 0;
	i32 height = 0;
	i32 pitch = 0;
	u8* memory = NULL;
};

struct _asset_font {
	// lots of font stuff
};

struct file_asset_font {

};

struct file_asset_bitmap {
	i32 width = 0;
	i32 height = 0;
	i32 pitch = 0;
	u64 memory = 0; // byte offset
};

struct file_asset {
	char name[128] = {};
	union {
		file_asset_bitmap bitmap;
		file_asset_font   font;
	};
	u64 next = 0; // byte offset;
};

struct asset {
	string name;
	asset_type type;
	union {
		_asset_bitmap 	bitmap;
		_asset_font 	font;
	};
};

struct asset_file_header {
	u32 num_assets = 0;
};

#ifndef BUILDER
struct asset_manager {
	map<string, asset> 	assets;
	vector<void*>		asset_stores;
	arena_allocator 	arena;
};

asset_manager make_asset_manager(allocator* a);
void destroy_asset_manager();

void load_asset_store(asset_manager* am, string path);
#endif
