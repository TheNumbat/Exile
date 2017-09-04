
#pragma once

template<typename T, u32 size>
struct buffer {

	u8 buf[size] = {};
	u32 used = 0;
	T param;

	void (*output)(T, void*, u32) = null;

///////////////////////////////////////////////////////////////////////////////

	static buffer make(void (*out)(T, void*, u32), T param);
	void destroy();

	void write(void* data, u32 len);
	void flush();
};

void write_file_wrapper(platform_file file, void* mem, u32 len);
