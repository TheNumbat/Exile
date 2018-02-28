
// don't care about using libraries in this; it's just to build the asset stores

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

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

#include "../engine/util/basic_types.h"
#include "../engine/asset.h"

struct def_asset_image {
	string name;
	string file;
};

struct def_asset_font {
	struct range {
		u32 start, end;
	};
	string name;
	string file;
	i32 point;
	i32 width, height;
	vector<range> ranges;
	bool write_out = false;
};

struct def_asset {
	asset_type type = asset_type::none;
	
	// can't union these...
	def_asset_image image;
	def_asset_font font;

	def_asset(const def_asset& source) {
		type = source.type;
		if(type == asset_type::bitmap) {
			image = source.image;
		} else if(type == asset_type::font) {
			font = source.font;
		}
	};
	def_asset() : type(), image(), font() {};
	~def_asset() {};
};

struct def_file_structure {
	vector<def_asset> assets;
};

bool control_char(char c) {
	return c == '\r' || c == '\n' || c == '\t' || c == ' ' || c == ':' || c == ',';
}

void eat_control(ifstream& in) {

	while(control_char((char)in.peek())) {
		in.get();
	}
}

def_file_structure build_def_file(ifstream& in) {

	def_file_structure ret;

	while(in.good()) {

		eat_control(in);
		string type;
		getline(in, type, '{');
		type.erase(type.find_last_not_of(" \n\r\t") + 1);

		if (!in.good()) break;

		def_asset asset;

		if(type == "image") {

			asset.type = asset_type::bitmap;
			eat_control(in);

			while(in.peek() != '}') {
				string field;
				in >> field;
				if(field == "name") {

					string name;
					eat_control(in);
					getline(in, name, ',');
					eat_control(in);
					name.erase(name.find_last_not_of(" \n\r\t") + 1);
					asset.image.name = name;

				} else if(field == "file") {

					string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.image.file = file;
				}
			}
			in.get();

		} else if(type == "font") {

			asset.type = asset_type::font;
			eat_control(in);

			while(in.peek() != '}') {

				string field;
				in >> field;
				if(field == "name") {

					string name;
					eat_control(in);
					getline(in, name, ',');
					eat_control(in);
					name.erase(name.find_last_not_of(" \n\r\t") + 1);
					asset.font.name = name;

				} else if(field == "write_out") {

					string write_out;
					eat_control(in);
					getline(in, write_out, ',');
					eat_control(in);
					write_out.erase(write_out.find_last_not_of(" \n\r\t") + 1);
					if(write_out == "true") {
						asset.font.write_out = true;
					}

				} else if(field == "file") {

					string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.font.file = file;

				} else if(field == "point") {

					i32 point;
					eat_control(in);
					in >> point;
					eat_control(in);
					asset.font.point = point;

				} else if(field == "width") {

					i32 width;
					eat_control(in);
					in >> width;
					eat_control(in);
					asset.font.width = width;

				} else if(field == "height") {

					i32 height;
					eat_control(in);
					in >> height;
					eat_control(in);
					asset.font.height = height;

				} else if(field == "range") {

					def_asset_font::range range;
					i32 start, end;
					eat_control(in);
					in >> start;
					eat_control(in);
					in >> end;
					eat_control(in);
					range.start = start;
					range.end = end;
					asset.font.ranges.push_back(range);
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
		cout << "You must pass an input and output file name!" << endl;
		return 1;
	}

	ifstream def_file(argv[1]);

	if(!def_file.good()) {
		cout << "Failed to open file " << argv[1] << endl;
		return 1;
	}

	ofstream assets_out(argv[2], ios::binary);
	
	if(!assets_out.good()) {
		cout << "Failed to create file " << argv[2] << endl;
		return 1;
	}

	string rel_path(argv[2]);
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

		} else if(def_asset.type == asset_type::font) {

			asset_header.type = asset_type::font;
			memcpy(asset_header.name, def_asset.font.name.c_str(), def_asset.font.name.size() + 1);			

			file_asset_font asset_font;

			ifstream font_in(rel_path + def_asset.font.file, ios::binary | ios::ate);
			streamsize size = font_in.tellg();
			font_in.seekg(0, ios::beg);

			vector<char> data((u32)size);
			font_in.read(data.data(), size);

			stbtt_fontinfo font_info;
			stbtt_pack_context pack_context;
			i32 ascent, descent, linegap;
			f32 scale;

			stbtt_InitFont(&font_info, (u8*)data.data(), 0);
			scale = stbtt_ScaleForPixelHeight(&font_info, (f32)def_asset.font.point);
			stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);

			u32 pixel_stride =  def_asset.font.width;
			u32 pixel_size = pixel_stride * def_asset.font.height;
			u8* baked_bitmap = (u8*)malloc(pixel_size);
			memset(baked_bitmap, 0, pixel_size);

			// Two pixel-padding needed for scaling (likely because of FP rounding error)
			stbtt_PackBegin(&pack_context, baked_bitmap, def_asset.font.width, def_asset.font.height, 0, 2, nullptr);
			stbtt_PackSetOversampling(&pack_context, 1, 1);

			u32 total_packedchars = 0;
			vector<stbtt_packedchar*> packedchars;	
			for(u32 ri = 0; ri < def_asset.font.ranges.size(); ri++) {
			
				i32 cp_num = def_asset.font.ranges[ri].end - def_asset.font.ranges[ri].start + 1;

				stbtt_packedchar* row = (stbtt_packedchar*)malloc(cp_num * sizeof(stbtt_packedchar));
				memset(row, 0, cp_num * sizeof(stbtt_packedchar));
				packedchars.push_back(row);
				total_packedchars += cp_num;

				stbtt_PackFontRange(&pack_context, (u8*)data.data(), 0, STBTT_POINT_SIZE((f32)def_asset.font.point), def_asset.font.ranges[ri].start, cp_num, packedchars[ri]);
			}
			stbtt_PackEnd(&pack_context);

			if(def_asset.font.write_out) {
				stbi_write_png((rel_path + def_asset.font.name + ".png").c_str(), def_asset.font.width, def_asset.font.height, 1, baked_bitmap, 0);
			}

			asset_font.num_glyphs 	= total_packedchars;
			asset_font.ascent 		= ascent * scale;
			asset_font.descent 		= descent * scale;
			asset_font.linegap 		= linegap * scale;
			asset_font.linedist 	= asset_font.ascent - asset_font.descent + asset_font.linegap;
			asset_font.width		= def_asset.font.width;
			asset_font.height		= def_asset.font.height;
			asset_font.point 		= (f32)def_asset.font.point;

			vector<file_glyph_data> glyph_data;
			for(u32 ri = 0; ri < def_asset.font.ranges.size(); ri++) {

				def_asset_font::range& r = def_asset.font.ranges[ri];
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

			asset_header.next = sizeof(file_asset_header) + sizeof(file_asset_font) + sizeof(file_glyph_data) * glyph_data.size() + pixel_size;

			assets_out.write((char*)&asset_header, sizeof(file_asset_header));
			assets_out.write((char*)&asset_font, sizeof(file_asset_font));
			assets_out.write((char*)glyph_data.data(), sizeof(file_glyph_data) * glyph_data.size());
			
			u32 out_size = pixel_size;
			u8* texture_out = (u8*)malloc(out_size);
			memset(texture_out, 0, out_size);
			u8* texture_out_place = texture_out;
			u8* bake_last = baked_bitmap + pixel_size - pixel_stride;
			for(; bake_last != baked_bitmap; bake_last -= pixel_stride) {
				for(u32 pix = 0; pix < pixel_stride; pix++) {
					*texture_out_place++ = bake_last[pix];
				}
			}
			assets_out.write((char*)texture_out, out_size);

			free(texture_out);
			free(baked_bitmap);
			for(stbtt_packedchar* pc : packedchars)
				free(pc);
		}
	}

	assets_out.close();

	cout << "Done!" << endl;

	return 0;
}
