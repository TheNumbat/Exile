
#pragma once

#include "basic.h"

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

enum class asset_type : u8 {
	none,
	bitmap,
	ttf_font,
	raster_font,
	shader
	// mesh?
	// audio?
	// cfg?
};

#pragma pack(push, 1)
struct file_glyph_data {
	u32 codepoint = 0; // sorted by this
	u16 x1        = 0;
	u16 y1        = 0;
	u16 x2        = 0;
	u16 y2        = 0; // texture coordinates
	f32 xoff1     = 0;
	f32 yoff1     = 0;
	f32 advance   = 0;
	f32 xoff2     = 0;
	f32 yoff2     = 0;
};

struct file_asset_raster_font {
	u32 num_glyphs = 0;
	f32 point      = 0;
	f32 ascent     = 0;
	f32 descent    = 0;
	f32 linegap    = 0;
	f32 linedist   = 0;
	i32 width      = 0;
	i32 height     = 0;
};

struct file_asset_ttf_font {};

struct file_asset_bitmap {
	i32 width 	= 0;
	i32 height 	= 0;
};

struct file_asset_header {
	asset_type type = asset_type::none;
	char name[128] 	= {};
	u64 next 		= 0; // byte offset from start of file_asset
};

struct asset_file_header {
	u32 num_assets = 0;
};
#pragma pack(pop)

#ifndef BUILDER

#include "ds/array.h"
#include "ds/alloc.h"
#include "ds/map.h"

typedef file_glyph_data glyph_data; // is this OK to be packed?

struct _asset_bitmap {
	i32 width = 0;
	i32 height = 0;
};

struct _asset_ttf_font {
	u64 file_size = 0;
};

struct _asset_raster_font {
	f32 point    = 0;
	f32 ascent   = 0;
	f32 descent  = 0;
	f32 linegap  = 0;
	f32 linedist = 0;
	i32 width    = 0;
	i32 height   = 0;
	array<file_glyph_data> glyphs;

///////////////////////////////////////////////////////////////////////////////

	glyph_data get_glyph(u32 codepoint);
};

struct asset {
	string name;
	asset_type type = asset_type::none;
	u8* mem = null;
	union {
		_asset_bitmap 		bitmap;
		_asset_raster_font  raster_font;
		_asset_ttf_font 	ttf_font;
	};
	asset() {}
};

struct asset_store {
	map<string, asset> 	assets;

	void* store = null;
	u32 store_sz = 0;
	
	string path;
	platform_file_attributes last;

	allocator* 			alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static asset_store make(allocator* a); // allocations are done in load
	void destroy();

	void load(string file);
	bool try_reload();
	asset* get(string name);

	glyph_data get_glyph(string font_asset_name, u32 codepoint);
};

#endif
