
#pragma once

template<typename U, typename V>
struct pair {

	pair(U& u, V& v) { first = u; second = v; }

	U first;
	V second;
};

template<typename U, typename V>
u32 hash(pair<U,V> p) {
	return hash(p.first) ^ hash(p.second);
}
