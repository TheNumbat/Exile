
#pragma once

#include "..\common.hpp"
#include "..\alloc.hpp"

#include <string>
#include <iostream>
#include <vector>

#include <tbb/concurrent_hash_map.h>

struct allocators_node_base {
	virtual ~allocators_node_base() {
		for(auto c : children)
			delete c.second;
	}
	tbb::concurrent_hash_map<std::string, allocators_node_base*> children;
	bool occupied;
	virtual size_t size() {return 0;}
	virtual size_t allocs() {return 0;}
	virtual std::string name() = 0;
};

template<typename T>
struct allocators_node : public allocators_node_base {
	alloc<T>* allocator;

	allocators_node(alloc<T>* a) {
		allocator = a;
		occupied = true;
	}
	size_t size() {
		return allocator->size;
	}
	std::string name() {
		std::string n = allocator->name;
		size_t pos = n.find_last_of("\\/");
		return n.substr(pos == std::string::npos ? 0 : pos + 1, n.size());
	}
	size_t allocs() {
		return allocator->num_allocs;
	}
};

template<>
struct allocators_node<void> : public allocators_node_base {
	allocators_node(std::string _n) {
		occupied = false;
		n = _n;
	}
	std::string name() {return n;}
	std::string n;
};

class debug {
public:
	debug() {
		allocators_root = new allocators_node<void>("ALLOCATORS");
	}
	~debug() {
		delete allocators_root;
	}

	void print_tree() {print_tree_rec(allocators_root, 0);}
	void print_tree_rec(allocators_node_base* current, int indent) {
		for(int i = 0; i < indent; i++)
			std::cerr << "\t";
		std::cerr << std::endl << current->name() << " " << current->size() << " " << current->allocs() << std::endl;
		for(auto& e : current->children)
			print_tree_rec(e.second, indent + 1);
	}

	template<typename T> void add_allocator(alloc<T>* a) {
		tbb::concurrent_hash_map<std::string, allocators_node_base*>::accessor acc;
		std::vector<std::string> n = split_name(a->name);
		allocators_node_base* current = allocators_root;
		for(unsigned int i = 0; i < n.size(); i++) {
			if(current->children.find(acc, n[i])) {
				current = acc->second;
			}
			else if(i == n.size() - 1) {
				allocators_node<T>* node = new allocators_node<T>(a);
				current->children.insert({n[i], node});
				current = node;
			} else {
				allocators_node<void>* node = new allocators_node<void>(n[i]);
				current->children.insert({n[i], node});
				current = node;
			}
		}
	}

	void rem_allocator(std::string name) {
		tbb::concurrent_hash_map<std::string, allocators_node_base*>::accessor acc;
		std::vector<std::string> n = split_name(name);
		allocators_node_base* current = allocators_root;
		for(unsigned int i = 0; i < n.size() - 1; i++) {
			bool success = current->children.find(acc, n[i]);
			assert(success);
			current = acc->second;
		}
		bool success = current->children.find(acc, n.back());
		assert(success);
		delete acc->second;
		current->children.erase(acc);
	}

private:
	std::vector<std::string> split_name(std::string name) {
		std::vector<std::string> ret;
		while(name.size()) {
			auto pos = name.find_first_of("\\/");
			if(pos == std::string::npos) {
				ret.push_back(name);
				break;
			}
			ret.push_back(name.substr(0, pos));
			name = name.substr(pos + 1, name.size());
		}
		return ret;
	}
	allocators_node_base* allocators_root;
};

extern debug gdebug;