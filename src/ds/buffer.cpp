
template<typename T, u32 size>
buffer<T,size> buffer<T,size>::make(void (*out)(T, void*, u32), T param) {

	buffer<T,size> ret;
	ret.output = out;
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
	
	memcpy(data, buf + used, len);
	used += len;
}

template<typename T, u32 size>
void buffer<T,size>::flush() {

	output(param, buf, used);
	used = 0;
}

void write_file_wrapper(platform_file file, void* mem, u32 len) {
	global_api->platform_write_file(&file, mem, len);
}
