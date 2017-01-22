
#pragma once

#include <string>
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

    alloc(std::string n) : name(n), size(0), num_allocs(0) {
        gdebug.add_allocator<T>(this);
    }
    alloc(const alloc& a) : name(a.name), size(a.size), num_allocs(a.num_allocs) {
        gdebug.rem_allocator(name);
        gdebug.add_allocator<T>(this);
    }
    template<typename U> alloc(const alloc<U>& a) : name(a.name), size(a.size), num_allocs(a.num_allocs) {
        gdebug.rem_allocator(name);
        gdebug.add_allocator<T>(this);   
    }

    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}

    pointer allocate(size_type n, const void* = 0) {
    	// replaceable with other malloc/free
        pointer p = static_cast<pointer>(scalable_malloc(n * sizeof(value_type)));
        if (!p)
            throw(std::bad_alloc());
        size += n * sizeof(value_type);
        num_allocs++;
        return p;
    }
    void deallocate(pointer p, size_type n) {
        scalable_free(p);
        size -= n * sizeof(value_type);
    }
    size_type max_size() const {
        size_type absolutemax = static_cast<size_type>(-1) / sizeof (value_type);
        return (absolutemax > 0 ? absolutemax : 1);
    }
    void construct(pointer p, value_type&& value) { 
    	new((void*)(p)) value_type(std::move(value)); 
    }
    void construct(pointer p, const value_type& value) { 
    	new((void*)(p)) value_type(value);
    }
    void destroy(pointer p) {
    	p->~value_type();
    }

	std::string name;
    size_type size;
    unsigned int num_allocs;

private:
	alloc& operator=(const alloc&);
};

template<>
class alloc<void> {
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;
    template<class U> struct rebind {
        typedef alloc<U> other;
    };
    alloc() {
        name = "VOID";
        size = 0;
    }
    std::string name;
    size_t size;
};