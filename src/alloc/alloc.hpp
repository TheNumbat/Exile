
#pragma once

#include <string>
#include <iostream>

#include <tbb/scalable_allocator.h>

// same as scalable_allocator but with names for debug system

template<typename T>
class alloc {
public:
	typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<class U> struct rebind {
        typedef alloc<U> other;
    };

    alloc(std::string n) throw() : name(n) {}
    alloc(const alloc& a) throw() : name(a.name) {}
    template<typename U> alloc(const alloc<U>& a) throw() : name(a.name) {}

    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}

    pointer allocate(size_type n, const void* = 0) {
        pointer p = static_cast<pointer>(scalable_malloc( n * sizeof(value_type)));
        if (!p)
            throw(std::bad_alloc());
        return p;
    }

    void deallocate(pointer p, size_type) {
        scalable_free(p);
    }

    size_type max_size() const throw() {
        size_type absolutemax = static_cast<size_type>(-1) / sizeof (value_type);
        return (absolutemax > 0 ? absolutemax : 1);
    }

    void construct(pointer p, value_type&& value) { ::new((void*)(p)) value_type( std::move( value ) ); }
    void construct(pointer p, const value_type& value) {::new((void*)(p)) value_type(value);}
    void destroy(pointer p) {p->~value_type();}

	std::string name;
};

