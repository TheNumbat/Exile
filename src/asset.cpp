
asset_store make_asset_store(allocator* a) {

	asset_store ret;

	ret.alloc = a;

	return ret;
}

void destroy_asset_store(asset_store* am) {

	if(am->store) {

		destroy_map(&am->assets);

		PUSH_ALLOC(am->alloc) {
			free(am->store);
		} POP_ALLOC();

		am->store = NULL;
	}
}

asset get_asset(asset_store* as, string name) {

	asset* a = map_try_get(&as->assets, name);

	if(!a) {
		LOG_ERR_F("Failed to get asset %s", name.c_str);
		asset ret;
		return ret;
	}

	return *a;
}

void load_asset_store(asset_store* am, string path) {

	platform_file store;
	platform_error err = global_state->api->platform_create_file(&store, path, open_file_existing);
	if(!err.good) {
		LOG_ERR_F("Failed to open asset store %s, error %u", path.c_str, err.error);
		global_state->api->platform_close_file(&store);
		return;
	}

	u32 store_size = global_state->api->platform_file_size(&store);

	PUSH_ALLOC(am->alloc) {

		am->store = malloc(store_size);
		u8* store_mem = (u8*)am->store;

		global_state->api->platform_read_file(&store, (void*)store_mem, store_size);
		global_state->api->platform_close_file(&store);

		asset_file_header* header = (asset_file_header*)store_mem;
		file_asset* current_asset = (file_asset*)(store_mem + sizeof(asset_file_header));

		am->assets = make_map<string,asset>(header->num_assets, am->alloc, &hash_string);

		for(u32 i = 0; i < header->num_assets; i++) {

			asset a;
			a.name = string_from_c_str(current_asset->name);

			if(current_asset->type == asset_bitmap) {

				a.type = asset_bitmap;

				a.bitmap.width = current_asset->bitmap.width;
				a.bitmap.height = current_asset->bitmap.height;
				a.bitmap.mem = (u8*)(current_asset) + sizeof(file_asset);

				map_insert(&am->assets, a.name, a);

				current_asset = (file_asset*)((u8*)current_asset + current_asset->next);

			} else {
				LOG_ERR("Only bitmaps for now!");
				break;
			}
		}

	} POP_ALLOC();
}
