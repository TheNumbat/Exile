
template<typename T, u32 size>
buffer<T,size> buffer<T,size>::make(_FPTR* out, T param) { 

	buffer<T,size> ret;
	ret.output.set(out);
	ret.param = param;

	return ret;
}

template<typename T, u32 size>
void buffer<T,size>::destroy() { 
	flush();
}

template<typename T, u32 size>
void buffer<T,size>::write(void* data, u32 len) { 

	if(len > size) {
		output(param, data, len);
		return;
	}

	if(len > size - used) {
		flush();
	}
	
	_memcpy(data, buf + used, len);
	used += len;
}

template<typename T, u32 size>
void buffer<T,size>::flush() { 

	if(used) {
	
		output(param, buf, used);
	}
	
	used = 0;
}

CALLBACK void write_file_wrapper(platform_file file, void* mem, u32 len) { 
	CHECKED(write_file, &file, mem, len);
}

CALLBACK void write_stdout_wrapper(u8 ignore, void* mem, u32 len) { 
	CHECKED(write_stdout, mem, len);
}
