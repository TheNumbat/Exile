
// don't care about using libraries in this; it's just to build the asset stores

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#pragma warning(disable : 4007)

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4456)
#pragma warning(disable : 4505)
#pragma warning(disable : 4996)
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#pragma warning(pop)

#define BUILDER

#include "../engine/asset.h"

struct def_asset_image {
	std::string name;
	std::string file;
};

struct def_asset_ttf_font {
	std::string name;
	std::string file;
};

struct def_asset_raster_font {
	struct range {
		u32 start, end;
	};
	std::string name;
	std::string file;
	i32 point;
	i32 width, height;
	std::vector<range> ranges;
	bool write_out = false;
};

struct def_asset {
	asset_type type = asset_type::none;
	
	// can't union these...
	def_asset_image 	  image;
	def_asset_raster_font raster_font;
	def_asset_ttf_font 	  ttf_font;

	def_asset(const def_asset& source) {
		type = source.type;
		image = source.image;
		raster_font = source.raster_font;
		ttf_font = source.ttf_font;
	};
	def_asset() : type(), image(), raster_font() {};
	~def_asset() {};
};

struct def_file_structure {
	std::vector<def_asset> assets;
};

bool control_char(char c) {
	return c == '\r' || c == '\n' || c == '\t' || c == ' ' || c == ':' || c == ',';
}

void eat_control(std::ifstream& in) {

	while(control_char((char)in.peek())) {
		in.get();
	}
}

def_file_structure build_def_file(std::ifstream& in) {

	def_file_structure ret;

	while(in.good()) {

		eat_control(in);
		std::string type;
		getline(in, type, '{');
		type.erase(type.find_last_not_of(" \n\r\t") + 1);

		if (!in.good()) break;

		def_asset asset;

		if(type == "image") {

			asset.type = asset_type::bitmap;
			eat_control(in);

			while(in.peek() != '}') {
				std::string field;
				in >> field;
				if(field == "name") {

					std::string name;
					eat_control(in);
					getline(in, name, ',');
					eat_control(in);
					name.erase(name.find_last_not_of(" \n\r\t") + 1);
					asset.image.name = name;

				} else if(field == "file") {

					std::string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.image.file = file;
				}
			}
			in.get();

		} else if(type == "raster_font") {

			asset.type = asset_type::raster_font;
			eat_control(in);

			while(in.peek() != '}') {

				std::string field;
				in >> field;
				if(field == "name") {

					std::string name;
					eat_control(in);
					getline(in, name, ',');
					eat_control(in);
					name.erase(name.find_last_not_of(" \n\r\t") + 1);
					asset.raster_font.name = name;

				} else if(field == "write_out") {

					std::string write_out;
					eat_control(in);
					getline(in, write_out, ',');
					eat_control(in);
					write_out.erase(write_out.find_last_not_of(" \n\r\t") + 1);
					if(write_out == "true") {
						asset.raster_font.write_out = true;
					}

				} else if(field == "file") {

					std::string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.raster_font.file = file;

				} else if(field == "point") {

					i32 point;
					eat_control(in);
					in >> point;
					eat_control(in);
					asset.raster_font.point = point;

				} else if(field == "width") {

					i32 width;
					eat_control(in);
					in >> width;
					eat_control(in);
					asset.raster_font.width = width;

				} else if(field == "height") {

					i32 height;
					eat_control(in);
					in >> height;
					eat_control(in);
					asset.raster_font.height = height;

				} else if(field == "range") {

					def_asset_raster_font::range range;
					i32 start, end;
					eat_control(in);
					in >> start;
					eat_control(in);
					in >> end;
					eat_control(in);
					range.start = start;
					range.end = end;
					asset.raster_font.ranges.push_back(range);
				}
			}
			in.get();

		} else if(type == "ttf_font") {

			asset.type = asset_type::ttf_font;
			eat_control(in);

			while(in.peek() != '}') {
				std::string field;
				in >> field;

				if(field == "name") {

					std::string name;
					eat_control(in);
					getline(in, name, ',');
					eat_control(in);
					name.erase(name.find_last_not_of(" \n\r\t") + 1);
					asset.ttf_font.name = name;

				} else if(field == "file") {

					std::string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.ttf_font.file = file;
				}
			}
			in.get();
		}

		ret.assets.push_back(asset);
	}

	return ret;
}

int main(int argc, char** argv) {

	if(argc < 3) {
		std::cout << "You must pass an input and output file name!" << std::endl;
		return 1;
	}

	std::ifstream def_file(argv[1]);

	if(!def_file.good()) {
		std::cout << "Failed to open file " << argv[1] << std::endl;
		return 1;
	}

	std::ofstream assets_out(argv[2], std::ios::binary);
	
	if(!assets_out.good()) {
		std::cout << "Failed to create file " << argv[2] << std::endl;
		return 1;
	}

	std::string rel_path(argv[1]);
	rel_path = rel_path.substr(0, rel_path.find_last_of("/\\") + 1);

	def_file_structure def = build_def_file(def_file);
	def_file.close();

	asset_file_header header;
	header.num_assets = (u32)def.assets.size();
	assets_out.write((char*)&header, sizeof(header));

	for(u32 i = 0; i < def.assets.size(); i++) {
		
		def_asset& 			def_asset = def.assets[i];
		file_asset_header 	asset_header;

		if(def_asset.type == asset_type::bitmap) {

			asset_header.type = asset_type::bitmap;
			memcpy(asset_header.name, def_asset.image.name.c_str(), def_asset.image.name.size() + 1);

			file_asset_bitmap asset_bitmap;

			// load bitmap
			i32 width, height;
			u8* bitmap = stbi_load((rel_path + def_asset.image.file).c_str(), &width, &height, nullptr, 4);

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

		} else if(def_asset.type == asset_type::ttf_font) {

 			asset_header.type = asset_type::ttf_font;
			memcpy(asset_header.name, def_asset.ttf_font.name.c_str(), def_asset.ttf_font.name.size() + 1);

			file_asset_ttf_font asset_font;

			std::ifstream font_in(rel_path + def_asset.ttf_font.file, std::ios::binary | std::ios::ate);
			std::streamsize file_size = font_in.tellg();
			font_in.seekg(0, std::ios::beg);

			std::vector<char> data((u32)file_size);
			font_in.read(data.data(), file_size);

			asset_header.next = sizeof(file_asset_header) + sizeof(file_asset_ttf_font) + file_size;

			assets_out.write((char*)&asset_header, sizeof(file_asset_header));
			assets_out.write((char*)&asset_font, sizeof(file_asset_ttf_font));

			assets_out.write((char*)data.data(), file_size);

		} else if(def_asset.type == asset_type::raster_font) {

			asset_header.type = asset_type::raster_font;
			memcpy(asset_header.name, def_asset.raster_font.name.c_str(), def_asset.raster_font.name.size() + 1);			

			file_asset_raster_font asset_font;

			std::ifstream font_in(rel_path + def_asset.raster_font.file, std::ios::binary | std::ios::ate);
			std::streamsize size = font_in.tellg();
			font_in.seekg(0, std::ios::beg);

			std::vector<char> data((u32)size);
			font_in.read(data.data(), size);

			stbtt_fontinfo font_info;
			stbtt_pack_context pack_context;
			i32 ascent, descent, linegap;
			f32 scale;

			stbtt_InitFont(&font_info, (u8*)data.data(), 0);
			scale = stbtt_ScaleForPixelHeight(&font_info, (f32)def_asset.raster_font.point);
			stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);

			u32 pixel_stride =  def_asset.raster_font.width;
			u32 pixel_size = pixel_stride * def_asset.raster_font.height;
			u8* baked_bitmap = new u8[pixel_size];
			memset(baked_bitmap, 0, pixel_size);

			// Two pixel-padding needed for scaling (likely because of FP rounding error)
			stbtt_PackBegin(&pack_context, baked_bitmap, def_asset.raster_font.width, def_asset.raster_font.height, 0, 2, nullptr);
			stbtt_PackSetOversampling(&pack_context, 1, 1);

			u32 total_packedchars = 0;
			std::vector<stbtt_packedchar*> packedchars;	
			for(u32 ri = 0; ri < def_asset.raster_font.ranges.size(); ri++) {
			
				i32 cp_num = def_asset.raster_font.ranges[ri].end - def_asset.raster_font.ranges[ri].start + 1;

				stbtt_packedchar* row = new stbtt_packedchar[cp_num];
				memset(row, 0, cp_num * sizeof(stbtt_packedchar));
				packedchars.push_back(row);
				total_packedchars += cp_num;

				stbtt_PackFontRange(&pack_context, (u8*)data.data(), 0, STBTT_POINT_SIZE((f32)def_asset.raster_font.point), def_asset.raster_font.ranges[ri].start, cp_num, packedchars[ri]);
			}
			stbtt_PackEnd(&pack_context);

			if(def_asset.raster_font.write_out) {
				stbi_write_png((rel_path + def_asset.raster_font.name + ".png").c_str(), def_asset.raster_font.width, def_asset.raster_font.height, 1, baked_bitmap, 0);
			}

			asset_font.num_glyphs 	= total_packedchars;
			asset_font.ascent 		= ascent * scale;
			asset_font.descent 		= descent * scale;
			asset_font.linegap 		= linegap * scale;
			asset_font.linedist 	= asset_font.ascent - asset_font.descent + asset_font.linegap;
			asset_font.width		= def_asset.raster_font.width;
			asset_font.height		= def_asset.raster_font.height;
			asset_font.point 		= (f32)def_asset.raster_font.point;

			std::vector<file_glyph_data> glyph_data;
			for(u32 ri = 0; ri < def_asset.raster_font.ranges.size(); ri++) {

				def_asset_raster_font::range& r = def_asset.raster_font.ranges[ri];
				for(u32 point = r.start; point <= r.end; point++) {

					file_glyph_data glyph;
					glyph.codepoint = point;
					glyph.x1      	= packedchars[ri][point - r.start].x0;
					glyph.y1      	= packedchars[ri][point - r.start].y0;
					glyph.x2      	= packedchars[ri][point - r.start].x1;
					glyph.y2      	= packedchars[ri][point - r.start].y1;
					glyph.xoff1   	= packedchars[ri][point - r.start].xoff;
					glyph.yoff1   	= packedchars[ri][point - r.start].yoff;
					glyph.xoff2   	= packedchars[ri][point - r.start].xoff2;
					glyph.yoff2   	= packedchars[ri][point - r.start].yoff2;
					glyph.advance 	= packedchars[ri][point - r.start].xadvance;

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
	}

	assets_out.close();

	return 0;
}
