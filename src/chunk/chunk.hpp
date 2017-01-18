
#pragma once

#include "..\common.hpp"
#include <array>

class chunk {
public:
	chunk();
	~chunk();

private:
	struct chunk_pos {
		s32 x, y, z;
	};
	struct block_pos {
		u8 x, y, z;
	};

private:
	// hash based or simply storage for all? .... going to be like half full
	// 64x64x256x5 = 5 mb per chunk -> 1gb is 205 chunks loaded (distance 14 or 896 blocks)
	// or let's say half that from hash map
	// storage vs speed? don't know...will go with speed for now

	// std::array<std::array<std::array<block, 256>, 64>, 64> blocks;
	// vs
	// std::unordered_map<block_pos, u16> blocks;
	
	chunk_pos pos;
};

