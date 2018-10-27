
#pragma once

typedef u16 block_type;

block_type block_bedrock;
block_type block_stone;
block_type block_path;
block_type block_stone_slab;
block_type block_torch;

struct world;

void init_blocks(world* w, asset_store* store);
