
#pragma once

template<typename T, typename A = Mdefault>
struct stack {
    vec<T,A> data;

    // note that a just-declared (i.e. zero'd) stack is already a valid stack
    static stack make(u32 capacity) {
        return {vec<T,A>::make(capacity)};
    }
    static stack copy(stack source) {
        return {vec<T,A>::copy(source.data)};
    }
    static stack take(stack& source) {
        return {vec<T,A>::take(source.data)};
    }

    void destroy() {
        data.destroy();
    }

    void push(T value) {
        data.push(value);
    }
    T pop() {
        return data.pop();
    }

    void clear() {
        data.clear();
    }

    const T* begin() const {
        return data.begin();
    }
    const T* end() const {
        return data.end();
    }
    T* begin() {
        return data.begin();
    }
    T* end() {
        return data.end();
    }
};

template<typename T, typename A> 
struct Type_Info<stack<T,A>> {
	static constexpr char name[] = "stack";
	static constexpr usize size = sizeof(stack<T,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    typedef stack<T,A> __offset;
	using members = Type_List<Record_Field<vec<T,A>,offsetof(__offset, data),_data>>;
};
