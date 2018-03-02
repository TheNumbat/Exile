
template<typename E>
void atomic_enum<E>::set(E val) {

	global_api->atomic_exchange(&value, (u64)val);
}

template<typename E>
E atomic_enum<E>::get() {

	return (E)value;
}
