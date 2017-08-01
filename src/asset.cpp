
asset_store make_asset_store(allocator* a) { PROF

	asset_store ret;

	ret.alloc = a;

	return ret;
}

void destroy_asset_store(asset_store* am) { PROF


	if(am->store) {

		destroy_map(&am->assets);

		PUSH_ALLOC(am->alloc) {
			free(am->store);
		} POP_ALLOC();

		am->store = null;
	}
}

asset* get_asset(asset_store* as, string name) { PROF

	asset* a = map_try_get(&as->assets, name);

	if(!a) {
		LOG_ERR_F("Failed to get asset %", name);
		return null;
	}

	return a;
}

glyph_data get_glyph_data(asset_store* as, string font, u32 codepoint) { 

#ifdef MORE_PROF
	PROF
#endif

	asset* a = get_asset(as, font);

	LOG_ASSERT(a->type == asset_type::font);

	return get_glyph_data(a, codepoint);
}

glyph_data get_glyph_data(asset* font, u32 codepoint) { 

#ifdef MORE_PROF
	PROF
#endif

	LOG_DEBUG_ASSERT(font->type == asset_type::font);

	u32 low = 0, high = font->font.glyphs.capacity;

	// binary search
	for(;;) {

		u32 search = low + ((high - low) / 2);

		glyph_data* data = array_get(&font->font.glyphs, search);

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

bool try_reload_asset_store(asset_store* as) { PROF

	platform_file_attributes new_attrib;
	
	global_state->api->platform_get_file_attributes(&new_attrib, as->path);	
	
	if(global_state->api->platform_test_file_written(&as->last, &new_attrib)) {

		PUSH_ALLOC(as->alloc) {

			free(as->store);
			as->store = null;

		} POP_ALLOC();

		map_clear(&as->assets);

		load_asset_store(as, as->path);

		LOG_INFO_F("Reloaded asset store from %", as->path);

		return true;
	}

	return false;
}

void load_asset_store(asset_store* as, string path) { PROF

	platform_file store;

	u32 itr = 0;
	platform_error err;
	do {
		itr++;
		err = global_state->api->platform_create_file(&store, path, platform_file_open_op::existing);
	} while(err.error == PLATFORM_SHARING_ERROR && itr < 100000);

	if(!err.good) {
		LOG_ERR_F("Failed to open asset store %, error %", path, err.error);
		global_state->api->platform_close_file(&store);
		return;
	}

	as->path = path;
	global_state->api->platform_get_file_attributes(&as->last, path);

	u32 store_size = global_state->api->platform_file_size(&store);

	PUSH_ALLOC(as->alloc) {

		as->store = malloc(store_size);
		u8* store_mem = (u8*)as->store;

		global_state->api->platform_read_file(&store, (void*)store_mem, store_size);
		global_state->api->platform_close_file(&store);

		asset_file_header* header = (asset_file_header*)store_mem;
		file_asset_header* current_asset = (file_asset_header*)(store_mem + sizeof(asset_file_header));

		as->assets = make_map<string,asset>(header->num_assets, as->alloc, &hash_string);

		for(u32 i = 0; i < header->num_assets; i++) {

			asset a;
			a.name = string_from_c_str(current_asset->name);

			if(current_asset->type == asset_type::bitmap) {

				a.type = asset_type::bitmap;

				file_asset_bitmap* bitmap = (file_asset_bitmap*)((u8*)current_asset + sizeof(file_asset_header));

				a.bitmap.width = bitmap->width;
				a.bitmap.height = bitmap->height;
				a.mem = (u8*)(current_asset) + sizeof(file_asset_header) + sizeof(file_asset_bitmap);

				map_insert(&as->assets, a.name, a);

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

				a.font.glyphs = make_array_memory<file_glyph_data>(font->num_glyphs, (u8*)font + sizeof(file_asset_font));

				a.mem = (u8*)font + sizeof(file_asset_font) + (font->num_glyphs * sizeof(file_glyph_data));

				map_insert(&as->assets, a.name, a);

				current_asset = (file_asset_header*)((u8*)current_asset + current_asset->next);				

			} else {

				LOG_ERR("Only bitmaps and fonts for now!");
				break;
			}

			LOG_DEBUG_F("Loaded asset % of type % from store %", a.name, a.type, path);
		}

	} POP_ALLOC();
}
