
#pragma once

// Asset file structure

// asset_file_header
// file_asset_header
// file_asset_bitmap/file_asset_font/etc
// memory for ^
// file_asset_header
// file_asset_bitmap/file_asset_font/etc
// memory for ^
// etc

// bitmap structure
// file_asset_bitmap
// bitmap memory

// font structure
// file_asset_font
// file_glyph_data[num_glyphs]
// bitmap memory

enum asset_type : u8 {
	asset_none,
	asset_bitmap,
	asset_font,
	// asset_mesh?
	// asset_audio?
	// asset_shader?
	// asset_cfg?
};

#pragma pack(push, 1)
struct file_glyph_data {
	u32 codepoint;		// sorted by this
	u16 x1, y1, x2, y2; // texture coordinates
	f32 xoff1, yoff1, advance, xoff2, yoff2;
};

struct file_asset_font {
	u32 num_glyphs;
	f32 point, ascent, descent, linegap, linedist;
	i32 width, height;
};

struct file_asset_bitmap {
	i32 width;
	i32 height;
};

struct file_asset_header {
	asset_type type;
	char name[128];
	u64 next; // byte offset from start of file_asset
};

struct asset_file_header {
	u32 num_assets;
};
#pragma pack(pop)

#ifndef BUILDER
typedef file_glyph_data glyph_data;

struct _asset_bitmap {
	i32 width = 0;
	i32 height = 0;
	u8* mem = NULL; // RGBA8888
};

struct _asset_font {
	f32 point, ascent, descent, linegap, linedist;
	i32 width, height;
	array<file_glyph_data> 	glyphs;
	u8* mem 		= NULL;
};

struct asset {
	string name;
	asset_type type = asset_none;
	union {
		_asset_bitmap 	bitmap;
		_asset_font 	font;
	};
	asset() : bitmap(), font() {};
};

struct asset_store {
	map<string, asset> 	assets;
	void* 				store = NULL;
	allocator* 			alloc;
};

asset_store make_asset_store(allocator* a); // allocations are done in load
void destroy_asset_store(asset_store* as);

void load_asset_store(asset_store* as, string path);
asset* get_asset(asset_store* as, string name);

glyph_data get_glyph_data(asset_store* as, string font, u32 codepoint);
glyph_data get_glyph_data(asset* font, u32 codepoint);
#endif
