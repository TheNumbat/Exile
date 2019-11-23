
#pragma once

// NOTE(max):
//      Remember to .reset() a non-static Varena when you are done with it.

void* base_alloc(usize sz);
void base_free(void* mem);

void* virt_alloc(usize sz);
void virt_free(void* mem);

void mem_validate();

template<const char* tname, bool log = true>
struct Mallocator {
    static constexpr const char* name = tname;

    template<typename T> 
    static T* make(usize n = 1) {
        return new (alloc<T>(sizeof(T) * n)) T[n];
    }

    template<typename T>
    static T* alloc(usize size);

    template<typename T>
    static void dealloc(T* mem);
};

template<const char* tname, bool log = true>
struct Mvallocator {
    static constexpr const char* name = tname;

    // NOTE(max): make() does not make sense here because if we are
    // creating a virtual allocation, we do not want to initalize it.

    template<typename T>
    static T* alloc(usize size);

    template<typename T>
    static void dealloc(T* mem);
};

static constexpr char Mdefault_name[] = "Mdefault";
using Mdefault = Mallocator<Mdefault_name>;
static constexpr char Mhidden_name[] = "Mhidden";
using Mhidden = Mallocator<Mhidden_name, false>;
static constexpr char Mvirtual_name[] = "Mvirtual";
using Mvirtual = Mallocator<Mvirtual_name>;

template<usize N>
struct Marena {
    u8 mem[N] = {};
    usize used = 0;
    usize high_water = 0;

    template<typename T> 
    T* make(usize n = 1) {
        T* ret = alloc<T>(sizeof(T) * n, alignof(T));
        for(usize i = 0; i < n; i++) {
            new (ret + i) T;
        }
        return ret;
    }

    template<typename T>
    void dealloc(T* mem) {}

    void reset() {
        used = 0;
    }    

private:
    template<typename T>
    T* alloc(usize size, usize align = 1);
};

template<const char* tname, usize N>
struct static_Marena {

    static constexpr const char* name = tname;
    static inline Marena<N> arena;

    template<typename T> 
    static T* make(usize n = 1) {return arena.template make<T>(n);}

    template<typename T>
    static T* alloc(usize size, usize align = 1) {return arena.alloc(size, align);}

    template<typename T>
    static void dealloc(T* mem) {}

    static void reset() {arena.reset();};
};

static constexpr char Mframe_name[] = "Mframe";
using Mframe = static_Marena<Mframe_name, MB(64)>;

template<typename U = Mvirtual, usize N = MB(16)>
struct Varena {
    u8* mem = null;
    usize used = 0;
    usize high_water = 0;

    template<typename T> 
    T* make(usize n = 1) {
        T* ret = alloc<T>(sizeof(T) * n, alignof(T));
        for(usize i = 0; i < n; i++) {
            new (ret + i) T;
        }
        return ret;
    }

    template<typename T>
    T* alloc(usize size, usize align = 1);

    template<typename T>
    void dealloc(T* mem) {}

    void init() {
        mem = U::template alloc<u8>(N);
    }
    void reset() {
        used = 0;
        U::dealloc(mem);
        mem = null;
    }    
};

template<const char* tname, usize N = MB(128)>
struct static_Varena {

    static Varena<Mvirtual, N> init() {return arena.init();}
    static void reset() {arena.reset();}

    static constexpr const char* name = tname;
    static inline Varena<Mvirtual, N> arena = (atexit(reset), init());

    template<typename T> 
    static T* make(usize n = 1) {return arena.template make<T>(n);}

    template<typename T>
    static T* alloc(usize size, usize align = 1) {return arena.alloc(size, align);}

    template<typename T>
    static void dealloc(T* mem) {}
};

template<const char* tname, typename T, typename Base>
struct Free_List {
    
    union Free_Node {
        T value;
        Free_Node* next = null;
    };
    
    static constexpr const char* name = tname;
    static inline Free_Node* list = null;

    static T* make() {
        return new (alloc()) T;
    }

    static T* alloc() {
        if(list) {
            Free_Node* ret = list;
            list = list->next;
            memset(ret, 0, sizeof(Free_Node));
            return (T*)ret;
        }
        return (T*)Base::template make<Free_Node>();
    }

    static void dealloc(T* mem) {
        Free_Node* node = (Free_Node*)mem;
        node->next = list;
        list = node;
    }

    static void clear() {
        while(list) {
            Free_Node* next = list->next;
            Base::dealloc(list);
            list = next;
        }
    }
};
