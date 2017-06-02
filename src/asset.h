
#pragma once

enum asset_type : u8 {
	asset_none,
	asset_bitmap,
	asset_font, 	// TODO(max)
	// asset_mesh?
	// asset_audio?
	// asset_shader?
	// asset_cfg?
};

struct _asset_bitmap {
	i32 width = 0;
	i32 height = 0;
	u8* mem = NULL; // RGBA8888
};

struct _asset_font {
	// lots of font stuff
};

#pragma pack(push, 1)
struct file_asset_font {

};

struct file_asset_bitmap {
	i32 width;
	i32 height;
};

struct file_asset {
	asset_type type;
	char name[128];
	union {
		file_asset_bitmap bitmap;
		file_asset_font   font;
	};
	u64 next; // byte offset from start of file_asset
};

struct asset_file_header {
	u32 num_assets;
};
#pragma pack(pop)

struct asset {
	string name;
	asset_type type = asset_none;
	union {
		_asset_bitmap 	bitmap;
		_asset_font 	font;
	};
	asset() : bitmap(), font() {};
};

#ifndef BUILDER
struct asset_store {
	map<string, asset> 	assets;
	void* 				store = NULL;
	allocator* 			alloc;
};

asset_store make_asset_store(allocator* a); // allocations are done in load
void destroy_asset_store(asset_store* as);

void load_asset_store(asset_store* as, string path);
asset get_asset(asset_store* as, string name);
#endif
