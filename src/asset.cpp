
asset_manager make_asset_manager(allocator* a) {

	asset_manager ret;

	ret.assets = make_map<string, asset>(1024, a, &hash_string);
	ret.stores = make_vector<void*>(8, a);
	ret.arena = MAKE_ARENA("assets", GIGABYTES(1), a, false);
	
	return ret;
}

void destroy_asset_manager(asset_manager* am) {

	destroy_map(&am->assets);

	PUSH_ALLOC(&am->arena) {

		for(u32 i = 0; i < am->stores.size; i++) {
			free(*vector_get(&am->stores, i));
		}

	} POP_ALLOC();

	destroy_vector(&am->stores);
	DESTROY_ARENA(&am->arena);
}

void load_asset_store(asset_manager* am, string path) {

	platform_file store;
	platform_error err = global_state->api->platform_create_file(&store, path, open_file_existing);
	if(!err.good) {
		LOG_ERR_F("Failed to open asset store %s, error %u", path.c_str, err.error);
		global_state->api->platform_close_file(&store);
		return;
	}

	PUSH_ALLOC(&am->arena) {

		u32 store_size = global_state->api->platform_file_size(&store);
		u8* store_mem = (u8*)malloc(store_size);
		vector_push(&am->stores, (void*)store_mem);
		global_state->api->platform_read_file(&store, (void*)store_mem, store_size);
		global_state->api->platform_close_file(&store);

		asset_file_header* header = (asset_file_header*)store_mem;
		file_asset* current_asset = (file_asset*)(store_mem + sizeof(asset_file_header));

		for(u32 i = 0; i < header->num_assets; i++) {

			asset a;
			a.name = string_from_c_str(current_asset->name);

			if(current_asset->type == asset_bitmap) {

				a.type = asset_bitmap;

				a.bitmap.width = current_asset->bitmap.width;
				a.bitmap.height = current_asset->bitmap.height;
				a.bitmap.mem = (u8*)(current_asset + sizeof(file_asset));

				map_insert(&am->assets, a.name, a);

				current_asset = (file_asset*)((u8*)current_asset + current_asset->next);

			} else {
				LOG_ERR("Only bitmaps for now!");
				break;
			}
		}

	} POP_ALLOC();
}
