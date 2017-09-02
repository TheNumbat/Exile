
asset_store asset_store::make(allocator* a) { PROF

	asset_store ret;

	ret.alloc = a;

	return ret;
}

void asset_store::destroy() { PROF

	if(store) {

		assets.destroy();

		PUSH_ALLOC(alloc) {
			free(store);
		} POP_ALLOC();

		store = null;
	}
}

asset* asset_store::get(string name) { PROF

	asset* a = assets.try_get(name);

	if(!a) {
		LOG_ERR_F("Failed to get asset %", name);
		return null;
	}

	return a;
}

glyph_data _asset_font::get_glyph(u32 codepoint) { 

#ifdef MORE_PROF
	PROF
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

#ifdef MORE_PROF
	PROF
#endif

	asset* a = get(font_asset_name);

	LOG_ASSERT(a->type == asset_type::font);

	return a->font.get_glyph(codepoint);
}

bool asset_store::try_reload() { PROF

	platform_file_attributes new_attrib;
	
	global_api->platform_get_file_attributes(&new_attrib, path);	
	
	if(global_api->platform_test_file_written(&last, &new_attrib)) {

		PUSH_ALLOC(alloc) {

			free(store);
			store = null;

		} POP_ALLOC();

		assets.clear();

		load(path);

		LOG_INFO_F("Reloaded asset store from %", path);

		return true;
	}

	return false;
}

void asset_store::load(string file) { PROF

	platform_file store_file;

	u32 itr = 0;
	platform_error err;
	do {
		itr++;
		err = global_api->platform_create_file(&store_file, file, platform_file_open_op::existing);
	} while(err.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!err.good) {
		LOG_ERR_F("Failed to open asset store %, error %", file, err.error);
		global_api->platform_close_file(&store_file);
		return;
	}

	path = file;
	global_api->platform_get_file_attributes(&last, file);

	u32 store_size = global_api->platform_file_size(&store_file);

	PUSH_ALLOC(alloc) {

		store = malloc(store_size);
		u8* store_mem = (u8*)store;

		global_api->platform_read_file(&store_file, (void*)store_mem, store_size);
		global_api->platform_close_file(&store_file);

		asset_file_header* header = (asset_file_header*)store_mem;
		file_asset_header* current_asset = (file_asset_header*)(store_mem + sizeof(asset_file_header));

		assets = map<string,asset>::make(header->num_assets, alloc, &hash_string);

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

				current_asset = (file_asset_header*)((u8*)current_asset + current_asset->next);

			} else if(current_asset->type == asset_type::font) {

				a.type = asset_type::font;

				file_asset_font* font = (file_asset_font*)((u8*)current_asset + sizeof(file_asset_header));

				a.font.ascent   = font->ascent;
				a.font.descent  = font->descent;
				a.font.linegap  = font->linegap;
				a.font.linedist = font->linedist;
				a.font.width    = font->width;
				a.font.height   = font->height;
				a.font.point 	= font->point;

				a.font.glyphs = array<file_glyph_data>::make_memory(font->num_glyphs, (u8*)font + sizeof(file_asset_font));

				a.mem = (u8*)font + sizeof(file_asset_font) + (font->num_glyphs * sizeof(file_glyph_data));

				assets.insert(a.name, a);

				current_asset = (file_asset_header*)((u8*)current_asset + current_asset->next);				

			} else {

				LOG_ERR("Only bitmaps and fonts for now!");
				break;
			}

			LOG_DEBUG_F("Loaded asset % of type % from store %", a.name, a.type, file);
		}

	} POP_ALLOC();
}
