
#pragma once

template<typename T, typename A = Mdefault>
struct stack {
    vec<T,A> data;

    // note that a just-declared (i.e. zero'd) stack is already a valid stack
    static stack<T,A> make(u32 capacity) {
        return {vec<T,A>::make(capacity)};
    }
    static stack<T,A> copy(stack<T,A> source) {
        return {vec<T,A>::copy(source.data)};
    }
    static stack<T,A> take(stack<T,A>& source) {
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
