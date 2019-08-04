
// don't care about using libraries in this; it's just to build the asset stores

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <picojson.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4456)
#pragma warning(disable : 4505)
#pragma warning(disable : 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define BUILDER
#include "../engine/asset.h"

std::string rel_path;
std::ofstream assets_out;

bool load_and_init(std::string def_file, std::string out_file, picojson::array& values) {

	std::ifstream input(def_file);
	if(!input.good()) {
		std::cout << "Failed to open def file " << def_file << std::endl;
		return false;
	}

	assets_out.open(out_file, std::ios::binary);
	if(!assets_out.good()) {
		std::cout << "Failed to create output file " << out_file << std::endl;
		return false;
	}

	rel_path = def_file.substr(0, def_file.find_last_of("/\\") + 1);

	std::string contents;
	getline(input, contents, (char)input.eof());
	input.close();

	picojson::value defs;
	std::string result = picojson::parse(defs, contents);
	if(!result.empty()) {
  		std::cout << "Failed to parse def file: " << result << std::endl;
  		return false;
	}

	values = defs.get<picojson::array>();

	asset_file_header header;
	header.num_assets = (u32)values.size();
	assets_out.write((char*)&header, sizeof(header));

	return true;
}

void output_image(picojson::value def) {

	std::string name = def.get("name").get<std::string>();
	std::string file = def.get("file").get<std::string>();

	file_asset_header asset_header;
	asset_header.type = (u8)asset_type::bitmap;
	memcpy(asset_header.name, name.c_str(), name.size() + 1);

	file_asset_bitmap asset_bitmap;

	// load bitmap
	i32 width, height;
	u8* bitmap = stbi_load((rel_path + file).c_str(), &width, &height, nullptr, 4);

	u32 pixel_stride = width * 4;
	u32 pixel_size = pixel_stride * height;

	asset_bitmap.width = width;
	asset_bitmap.height = height;
	asset_header.next = sizeof(file_asset_header) + sizeof(file_asset_bitmap) + pixel_size;

	assets_out.write((char*)&asset_header, sizeof(file_asset_header));
	assets_out.write((char*)&asset_bitmap, sizeof(file_asset_bitmap));

	u8* pixel_last = bitmap + pixel_size - pixel_stride;
	for(; pixel_last != bitmap; pixel_last -= pixel_stride) {
		u8* pixel_out_place = pixel_last;
		for(u32 pix = 0; pix < pixel_stride; pix += 4) {
			*pixel_out_place++ = (u8)roundf(pixel_last[pix + 0] * (pixel_last[pix + 3] / 255.0f));
			*pixel_out_place++ = (u8)roundf(pixel_last[pix + 1] * (pixel_last[pix + 3] / 255.0f));
			*pixel_out_place++ = (u8)roundf(pixel_last[pix + 2] * (pixel_last[pix + 3] / 255.0f));
			*pixel_out_place++ = pixel_last[pix + 3];
		}
		assets_out.write((char*)pixel_last, pixel_stride);
	}
	assets_out.write((char*)pixel_last, pixel_stride);


	stbi_image_free(bitmap);
}

void output_font(picojson::value def) {

	std::string name = def.get("name").get<std::string>();
	std::string file = def.get("file").get<std::string>();

	file_asset_header asset_header;
	asset_header.type = (u8)asset_type::ttf_font;
	memcpy(asset_header.name, name.c_str(), name.size() + 1);

	file_asset_ttf_font asset_font;

	std::ifstream font_in(rel_path + file, std::ios::binary | std::ios::ate);
	std::streamsize file_size = font_in.tellg();
	font_in.seekg(0, std::ios::beg);

	std::vector<char> data((u32)file_size);
	font_in.read(data.data(), file_size);

	asset_header.next = sizeof(file_asset_header) + sizeof(file_asset_ttf_font) + file_size;

	assets_out.write((char*)&asset_header, sizeof(file_asset_header));
	assets_out.write((char*)&asset_font, sizeof(file_asset_ttf_font));

	assets_out.write((char*)data.data(), file_size);
}

void output_raster_font(picojson::value def) {

	std::string name = def.get("name").get<std::string>();
	std::string file = def.get("file").get<std::string>();
	f32 point = (f32)def.get("point").get<double>();
	u32 width = (u32)def.get("width").get<double>();
	u32 height = (u32)def.get("height").get<double>();
	picojson::array ranges = def.get("ranges").get<picojson::array>();

	bool write_out = false;
	if(def.contains("write_out")) {
		write_out = def.get("write_out").get<bool>();
	}

	file_asset_header asset_header;
	asset_header.type = (u8)asset_type::raster_font;
	memcpy(asset_header.name, name.c_str(), name.size() + 1);			

	file_asset_raster_font asset_font;

	std::ifstream font_in(rel_path + file, std::ios::binary | std::ios::ate);
	std::streamsize size = font_in.tellg();
	font_in.seekg(0, std::ios::beg);

	std::vector<char> data((u32)size);
	font_in.read(data.data(), size);

	stbtt_fontinfo font_info;
	stbtt_pack_context pack_context;
	i32 ascent, descent, linegap;
	f32 scale;

	stbtt_InitFont(&font_info, (u8*)data.data(), 0);
	scale = stbtt_ScaleForPixelHeight(&font_info, point);
	stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);

	u32 pixel_stride = width;
	u32 pixel_size = pixel_stride * height;
	u8* baked_bitmap = new u8[pixel_size];
	memset(baked_bitmap, 0, pixel_size);

	// Two pixel-padding needed for scaling (likely because of FP rounding error)
	stbtt_PackBegin(&pack_context, baked_bitmap, width, height, 0, 2, nullptr);
	stbtt_PackSetOversampling(&pack_context, 1, 1);

	u32 total_packedchars = 0;
	std::vector<stbtt_packedchar*> packedchars;
	for(picojson::value v : ranges) {
		
		i32 start = (i32)v.get(0).get<double>();
		i32 end = (i32)v.get(1).get<double>();
		i32 cp_num = end - start + 1;

		stbtt_packedchar* row = new stbtt_packedchar[cp_num];
		memset(row, 0, cp_num * sizeof(stbtt_packedchar));
		packedchars.push_back(row);
		total_packedchars += cp_num;

		stbtt_PackFontRange(&pack_context, (u8*)data.data(), 0, STBTT_POINT_SIZE((f32)point), start, cp_num, packedchars.back());
	}
	stbtt_PackEnd(&pack_context);

	if(write_out) {
		stbi_write_png((rel_path + name + ".png").c_str(), width, height, 1, baked_bitmap, 0);
	}

	asset_font.num_glyphs 	= total_packedchars;
	asset_font.ascent 		= ascent * scale;
	asset_font.descent 		= descent * scale;
	asset_font.linegap 		= linegap * scale;
	asset_font.linedist 	= asset_font.ascent - asset_font.descent + asset_font.linegap;
	asset_font.width		= width;
	asset_font.height		= height;
	asset_font.point 		= (f32)point;

	std::vector<file_glyph_data> glyph_data;
	for(u32 ri = 0; ri < ranges.size(); ri++) {

		i32 start = (i32)ranges[ri].get(0).get<double>();
		i32 end = (i32)ranges[ri].get(1).get<double>();

		for(i32 cp = start; cp <= end; cp++) {

			file_glyph_data glyph;
			glyph.codepoint = cp;
			glyph.x1      	= packedchars[ri][cp - start].x0;
			glyph.y1      	= packedchars[ri][cp - start].y0;
			glyph.x2      	= packedchars[ri][cp - start].x1;
			glyph.y2      	= packedchars[ri][cp - start].y1;
			glyph.xoff1   	= packedchars[ri][cp - start].xoff;
			glyph.yoff1   	= packedchars[ri][cp - start].yoff;
			glyph.xoff2   	= packedchars[ri][cp - start].xoff2;
			glyph.yoff2   	= packedchars[ri][cp - start].yoff2;
			glyph.advance 	= packedchars[ri][cp - start].xadvance;

			glyph_data.push_back(glyph);
		}
	}
	sort(glyph_data.begin(), glyph_data.end(), [](file_glyph_data& one, file_glyph_data& two) -> bool {return one.codepoint < two.codepoint;});

	asset_header.next = sizeof(file_asset_header) + sizeof(file_asset_raster_font) + sizeof(file_glyph_data) * glyph_data.size() + pixel_size;

	assets_out.write((char*)&asset_header, sizeof(file_asset_header));
	assets_out.write((char*)&asset_font, sizeof(file_asset_raster_font));
	assets_out.write((char*)glyph_data.data(), sizeof(file_glyph_data) * glyph_data.size());
	
	u32 out_size = pixel_size;
	u8* texture_out = new u8[out_size];
	memset(texture_out, 0, out_size);
	u8* texture_out_place = texture_out;
	u8* bake_last = baked_bitmap + pixel_size - pixel_stride;
	for(; bake_last != baked_bitmap; bake_last -= pixel_stride) {
		for(u32 pix = 0; pix < pixel_stride; pix++) {
			*texture_out_place++ = bake_last[pix];
		}
	}
	assets_out.write((char*)texture_out, out_size);

	delete[] texture_out;
	delete[] baked_bitmap;
	for(stbtt_packedchar* pc : packedchars)
		delete[] pc;
}

int main(int argc, char** argv) {

	if(argc < 3) {
		std::cout << "You must pass an input and output file name!" << std::endl;
		return 1;
	}


	picojson::array assets;
	if(!load_and_init(argv[1],argv[2],assets)) return 1;

	for(picojson::value v : assets) {
		
		std::string type = v.get("type").get<std::string>();

		if(type == "image") {

			output_image(v);

		} else if(type == "font") {

			output_font(v);

		} else if(type == "raster_font") {

			output_raster_font(v);
		}
	}

	assets_out.close();

	return 0;
}
