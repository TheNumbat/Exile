

asset_store asset_store::make(allocator* a) { 

	asset_store ret;

	ret.alloc = a;
	ret.assets = map<string,asset>::make(64, ret.alloc);

	return ret;
}

void asset_store::destroy() { 

	path.destroy(alloc);
	assets.destroy();

	if(store) {

		PUSH_ALLOC(alloc) {
			free(store, store_sz);
		} POP_ALLOC();

		store = null;
	}
}

asset* asset_store::get(string name) { 

	asset* a = assets.try_get(name);

	if(!a) {
		LOG_ERR_F("Failed to get asset %"_, name);
		return null;
	}

	return a;
}

glyph_data _asset_raster_font::get_glyph(u32 codepoint) { 

#ifdef MORE_
	
#endif

	u32 low = 0, high = glyphs.capacity;

	// binary search
	for(;;) {

		u32 search = low + ((high - low) / 2);

		glyph_data* data = glyphs.get(search);

		if(data->codepoint == codepoint) {
			return *data;
		}

		if(data->codepoint < codepoint) {
			low = search + 1;
		} else {
			high = search;
		}

		if(low == high) {
			glyph_data ret;
			return ret;
		}
	}
}

glyph_data asset_store::get_glyph(string font_asset_name, u32 codepoint) { 

	asset* a = get(font_asset_name);

	LOG_ASSERT(a->type == asset_type::raster_font);

	return a->raster_font.get_glyph(codepoint);
}

bool asset_store::try_reload() { 

	platform_file_attributes new_attrib;
	
	CHECKED(get_file_attributes, &new_attrib, path);	
	
	if(global_api->test_file_written(&last, &new_attrib)) {

		PUSH_ALLOC(alloc) {

			free(store, store_sz);
			store = null;

		} POP_ALLOC();

		assets.clear();

		load(path);

		LOG_DEBUG_F("Reloaded asset store from %"_, path);

		return true;
	}

	return false;
}

void asset_store::load(string file) { 

	platform_file store_file;

	u32 itr = 0;
	platform_error error;
	do {
		itr++;
		error = global_api->create_file(&store_file, file, platform_file_open_op::existing);
	} while(error.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!error.good) {
		LOG_ERR_F("Failed to open asset store %, error %"_, file, error.error);
		CHECKED(close_file, &store_file);
		return;
	}

	PUSH_ALLOC(alloc) {
	
		if(path.c_str) path.destroy();
		path = string::make_copy(file);
		CHECKED(get_file_attributes, &last, file);

		store_sz = global_api->file_size(&store_file);

		store = malloc(store_sz);
		u8* store_mem = (u8*)store;

		CHECKED(read_file, &store_file, (void*)store_mem, store_sz);
		CHECKED(close_file, &store_file);

		asset_file_header* header = (asset_file_header*)store_mem;
		file_asset_header* current_asset = (file_asset_header*)(store_mem + sizeof(asset_file_header));

		for(u32 i = 0; i < header->num_assets; i++) {

			asset a;
			a.name = string::from_c_str(current_asset->name);

			if(current_asset->type == asset_type::bitmap) {

				a.type = asset_type::bitmap;

				file_asset_bitmap* bitmap = (file_asset_bitmap*)((u8*)current_asset + sizeof(file_asset_header));

				a.bitmap.width = bitmap->width;
				a.bitmap.height = bitmap->height;
				a.mem = (u8*)(current_asset) + sizeof(file_asset_header) + sizeof(file_asset_bitmap);

				assets.insert(a.name, a);

			} else if(current_asset->type == asset_type::ttf_font) {

				a.type = asset_type::ttf_font;

				// yes this is useless
				file_asset_ttf_font* font = (file_asset_ttf_font*)((u8*)current_asset + sizeof(file_asset_header));

				a.mem = (u8*)font + sizeof(file_asset_ttf_font);
				a.ttf_font.file_size = ((u8*)current_asset + current_asset->next) - a.mem;

				assets.insert(a.name, a);

			} else if(current_asset->type == asset_type::raster_font) {

				a.type = asset_type::raster_font;

				file_asset_raster_font* font = (file_asset_raster_font*)((u8*)current_asset + sizeof(file_asset_header));

				a.raster_font.ascent   = font->ascent;
				a.raster_font.descent  = font->descent;
				a.raster_font.linegap  = font->linegap;
				a.raster_font.linedist = font->linedist;
				a.raster_font.width    = font->width;
				a.raster_font.height   = font->height;
				a.raster_font.point    = font->point;

				a.raster_font.glyphs = array<file_glyph_data>::make_memory(font->num_glyphs, (u8*)font + sizeof(file_asset_raster_font));

				a.mem = (u8*)font + sizeof(file_asset_raster_font) + (font->num_glyphs * sizeof(file_glyph_data));

				assets.insert(a.name, a);

			} else {

				LOG_ERR_F("Unrecognized asset type %!"_, current_asset->type);
				break;
			}

			LOG_DEBUG_F("Loaded asset % of type % from store %"_, a.name, a.type, file);
			current_asset = (file_asset_header*)((u8*)current_asset + current_asset->next);
		}

	} POP_ALLOC();
}
