
// don't care about using libraries in this; it's just to build the asset stores
// we don't even use the stuff for the game
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4456)
#pragma warning(disable : 4505)
#pragma warning(disable : 4996)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#pragma warning(pop)

#include "basic_types.h"
#define BUILDER
#include "asset.h"

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
	i32 size;
	vector<range> ranges;
};

struct def_asset {
	asset_type type;
	union {
		def_asset_image image;
		def_asset_font font;
	};
	def_asset(const def_asset& source) {memcpy(this, &source, sizeof(def_asset));};
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

			asset.type = asset_bitmap;
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

			asset.type = asset_font;
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

				} else if(field == "file") {

					string file;
					eat_control(in);
					getline(in, file, ',');
					eat_control(in);
					file.erase(file.find_last_not_of(" \n\r\t") + 1);
					asset.font.file = file;

				} else if(field == "size") {

					i32 size;
					eat_control(in);
					in >> size;
					eat_control(in);
					asset.font.size = size;

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

	def_file_structure def = build_def_file(def_file);
	def_file.close();

	asset_file_header header;
	header.num_assets = (u32)def.assets.size();
	assets_out.write((char*)&header, sizeof(header));

	for(i32 i = 0; i < def.assets.size(); i++) {
		
		def_asset& 			def_asset = def.assets[i];
		file_asset_header 	asset_header;

		if(def_asset.type == asset_bitmap) {

			asset_header.type = asset_bitmap;
			memcpy(asset_header.name, def_asset.image.name.c_str(), def_asset.image.name.size() + 1);

			file_asset_bitmap asset_bitmap;

			// load bitmap
			i32 width, height;
			u8* bitmap = stbi_load(def_asset.image.file.c_str(), &width, &height, NULL, 4);

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

		} else if(def_asset.type == asset_font) {

			asset_header.type = asset_font;
			memcpy(asset_header.name, def_asset.font.name.c_str(), def_asset.font.name.size() + 1);			

			file_asset_font asset_font;

			ifstream font_in(def_asset.font.file, ios::binary | ios::ate);
			streamsize size = font_in.tellg();
			font_in.seekg(0, ios::beg);

			vector<char> data(size);
			font_in.read(data.data(), size);

			stbtt_fontinfo font_info;
			stbtt_pack_context pack_context;
			i32 ascent, descent, baseline, linegap;
			f32 scale;

			stbtt_InitFont(&font_info, (u8*)data.data(), 0);
			scale = stbtt_ScaleForPixelHeight(&font_info, (f32)def_asset.font.size);
			stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);
			baseline = (i32) (ascent * scale);

			u32 pixel_stride =  1024 * 1;
			u32 pixel_size = 1024 * pixel_stride;
			u8* baked_bitmap = (u8*)malloc(pixel_size);

			stbtt_PackBegin(&pack_context, baked_bitmap, 1024, 1024, 0, 1, NULL);
			stbtt_PackSetOversampling(&pack_context, 1, 1);

			u32 total_packedchars_size = 0, total_packedchars = 0;
			vector<stbtt_packedchar*> packedchars;	
			vector<i32> 		      cp_nums;
			for(i32 ri = 0; ri < def_asset.font.ranges.size(); ri++) {
			
				i32 cp_num = def_asset.font.ranges[ri].end - def_asset.font.ranges[ri].start;

				packedchars.push_back((stbtt_packedchar*)malloc(cp_num * sizeof(stbtt_packedchar)));
				total_packedchars_size += cp_num * sizeof(stbtt_packedchar);
				total_packedchars += cp_num;
				cp_nums.push_back(cp_num);

				stbtt_PackFontRange(&pack_context, (u8*)data.data(), 0, (f32)def_asset.font.size, def_asset.font.ranges[ri].start, cp_num, packedchars[ri]);
			}
			stbtt_PackEnd(&pack_context);

			asset_font.num_glyphs 	= total_packedchars;
			asset_font.baseline 	= baseline;
			asset_font.ascent 		= ascent;
			asset_font.descent 		= descent;
			asset_font.linegap 		= linegap;
			asset_font.linedist 	= ascent - descent + linegap;
			asset_header.next 		= sizeof(file_asset_header) + sizeof(file_asset_font) +  + pixel_size * 4;

			assets_out.write((char*)&asset_header, sizeof(file_asset_header));
			assets_out.write((char*)&asset_font, sizeof(file_asset_font));
			for(i32 pc_i = 0; pc_i < packedchars.size(); pc_i++)
				assets_out.write((char*)packedchars[pc_i], cp_nums[pc_i] * sizeof(stbtt_packedchar));

			u8* texture_out = (u8*)malloc(1024 * 1024 * 4);
			u8* texture_out_place = texture_out;
			u8* bake_last = baked_bitmap + pixel_size - pixel_stride;
			for(; bake_last != baked_bitmap; bake_last -= pixel_stride) {
				for(i32 pix = 0; pix < 1024; pix++) {
					*texture_out_place++ = bake_last[pix];
					*texture_out_place++ = bake_last[pix];
					*texture_out_place++ = bake_last[pix];
					*texture_out_place++ = bake_last[pix];
				}
			}
			assets_out.write((char*)texture_out, 1024 * 1024 * 4);

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
