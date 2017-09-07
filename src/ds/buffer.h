
#pragma once

template<typename T, u32 size>
struct buffer {

	u8 buf[size] = {};
	u32 used = 0;
	T param;

	func_ptr<void, T, void*, u32> output;

///////////////////////////////////////////////////////////////////////////////

	static buffer make(_FPTR out, T param);
	void destroy();

	void write(void* data, u32 len);
	void flush();
};

CALLBACK void write_file_wrapper(platform_file file, void* mem, u32 len);
