	
i64 int_as_i64(void* val, _type_info* info) {

	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			return (i64)*(i8*)val;
		} else {
			return (i64)*(u8*)val;
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			return (i64)*(i16*)val;
		} else {
			return (i64)*(u16*)val;
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			return (i64)*(i32*)val;
		} else {
			return (i64)*(u32*)val;
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			return (i64)*(i64*)val;
		} else {
			return (i64)*(u64*)val;
		}
	} break;
	default: {
		INVALID_PATH;
		return 0;
	} break;
	}
}
