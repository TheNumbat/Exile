
void init_blocks(world* w, asset_store* store) {

	texture_id tex = w->block_textures;
	i32 tex_idx = eng->ogl.get_layers(tex);
	
	tex_idx = eng->ogl.get_layers(tex);
	eng->ogl.push_tex_array(tex, store, "bedrock"_);

	block_meta* bedrock = w->add_block();
	block_bedrock = bedrock->type;
	*bedrock = {
		block_bedrock,
		{true, true, true, true, true, true},
		{tex_idx, tex_idx, tex_idx, tex_idx, tex_idx, tex_idx},
		{block_bedrock, block_bedrock, block_bedrock, block_bedrock, block_bedrock, block_bedrock},
		true, true, false
	};


	tex_idx = eng->ogl.get_layers(tex);
	eng->ogl.push_tex_array(tex, store, "stone"_);

	block_meta* stone = w->add_block();
	block_stone = stone->type;
	*stone = {
		block_stone,
		{true, true, true, true, true, true},
		{tex_idx, tex_idx, tex_idx, tex_idx, tex_idx, tex_idx},
		{block_stone, block_stone, block_stone, block_stone, block_stone, block_stone},
		true, true, false
	};


	tex_idx = eng->ogl.get_layers(tex);
	eng->ogl.push_tex_array(tex, store, "path_side"_);
	eng->ogl.push_tex_array(tex, store, "dirt"_);
	eng->ogl.push_tex_array(tex, store, "path_top"_);
	
	block_meta* path = w->add_block();
	block_path = path->type;
	*path = {
		block_path,
		{true, true, true, true, true, true},
		{tex_idx, tex_idx + 1, tex_idx, tex_idx, tex_idx + 2, tex_idx},
		{block_path, block_path, block_path, block_path, block_path, block_path},
		true, true, false
	};	


	tex_idx = eng->ogl.get_layers(tex);
	eng->ogl.push_tex_array(tex, store, "slab_side"_);
	eng->ogl.push_tex_array(tex, store, "slab_top"_);

	block_meta* stone_slab = w->add_block();
	block_stone_slab = stone_slab->type;
	*stone_slab = {
		block_stone_slab,
		{false, true, false, false, false, false},
		{tex_idx, tex_idx + 1, tex_idx, tex_idx, tex_idx + 1, tex_idx},
		{0, block_stone_slab, block_stone_slab, block_stone_slab, block_stone_slab, 0},
		true, false, true, FPTR(slab_model)
	};	
}
