
#pragma once

template<typename T> struct v2_base;
template<typename T> struct v3_base;
template<typename T> struct v4_base;
template<typename T, usize N> struct vN_base;

template<typename T, usize N>
using vect_base = typename Conditional<N == 2, v2_base<T>, 
                  typename Conditional<N == 3, v3_base<T>, 
                  typename Conditional<N == 4, v4_base<T>, 
                  vN_base<T,N>>::type>::type>::type;

template<typename T, usize N>
struct vect : vect_base<T,N> {

    using base = vect_base<T,N>;
    static constexpr bool is_simd = N == 4 && Is_Same<T,f32>::value;
    static_assert(N >= 2, "Why use a 1-sized vect?");

    vect() : base{} {};
    vect(__m128 p) : base{} {static_assert(is_simd, "Not a SIMD vec!");
        this->pack = p;}
    vect(T x) : base{x} {}
    vect(T x, T y) : base{x,y} {}
    vect(T x, T y, T z) : base{x,y,z} {}
    vect(T x, T y, T z, T w) : base{x,y,z,w} {}
    template<typename... Ts> 
    vect(Ts... list) : base{list...} {static_assert(sizeof...(Ts) == N, "Too few/many arguments!");}

	i32& operator[](T idx) {return this->data[idx];}
    i32 operator[](T idx) const {return this->data[idx];}

	vect operator+=(vect v) {
        if constexpr(is_simd) this->pack = _mm_add_ps(this->pack, v.pack);
        else for(usize i = 0; i < N; i++) this->data[i] += v.data[i]; return *this;}
	vect operator-=(vect v) {
        if constexpr(is_simd) this->pack = _mm_sub_ps(this->pack, v.pack);
        else for(usize i = 0; i < N; i++) this->data[i] -= v.data[i]; return *this;}
	vect operator*=(vect v) {
        if constexpr(is_simd) this->pack = _mm_mul_ps(this->pack, v.pack);
        else for(usize i = 0; i < N; i++) this->data[i] *= v.data[i]; return *this;}
	vect operator/=(vect v) {
        if constexpr(is_simd) this->pack = _mm_div_ps(this->pack, v.pack);
        else for(usize i = 0; i < N; i++) this->data[i] /= v.data[i]; return *this;}

	vect operator+=(T s) {
        if constexpr(is_simd) this->pack = _mm_add_ps(this->pack, _mm_set1_ps(s));
        else for(usize i = 0; i < N; i++) this->data[i] += s; return *this;}
	vect operator-=(T s) {
        if constexpr(is_simd) this->pack = _mm_sub_ps(this->pack, _mm_set1_ps(s));
        else for(usize i = 0; i < N; i++) this->data[i] -= s; return *this;}
	vect operator*=(T s) {
        if constexpr(is_simd) this->pack = _mm_mul_ps(this->pack, _mm_set1_ps(s));
        else for(usize i = 0; i < N; i++) this->data[i] *= s; return *this;}
	vect operator/=(T s) {
        if constexpr(is_simd) this->pack = _mm_div_ps(this->pack, _mm_set1_ps(s));
        else for(usize i = 0; i < N; i++) this->data[i] /= s; return *this;}

    vect operator+(vect o) const {
        if constexpr(is_simd) return {_mm_add_ps(this->pack, o.pack)};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] + o.data[i]; return r;}}
    vect operator-(vect o) const {
        if constexpr(is_simd) return {_mm_sub_ps(this->pack, o.pack)};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] - o.data[i]; return r;}}
    vect operator*(vect o) const {
        if constexpr(is_simd) return {_mm_mul_ps(this->pack, o.pack)};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] * o.data[i]; return r;}}
    vect operator/(vect o) const {
        if constexpr(is_simd) return {_mm_div_ps(this->pack, o.pack)};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] / o.data[i]; return r;}}

    vect operator+(T s) const {
        if constexpr(is_simd) return {_mm_add_ps(this->pack, _mm_set1_ps(s))};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] + s; return r;}}
    vect operator-(T s) const {
        if constexpr(is_simd) return {_mm_sub_ps(this->pack, _mm_set1_ps(s))};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] - s; return r;}}
    vect operator*(T s) const {
        if constexpr(is_simd) return {_mm_mul_ps(this->pack, _mm_set1_ps(s))};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] * s; return r;}}
    vect operator/(T s) const {
        if constexpr(is_simd) return {_mm_div_ps(this->pack, _mm_set1_ps(s))};
        else {vect r; for(usize i = 0; i < N; i++) r.data[i] = this->data[i] / s; return r;}}

    bool operator==(vect o) const {
        if constexpr(is_simd) return _mm_movemask_ps(_mm_cmpeq_ps(this->pack, o.pack)) == 0xf;
        else { bool good = true;
            for(usize i = 0; good && i < N; i++) good = good && this->data[i] == o.data[i];
            return good;
        }
    }
    bool operator!=(vect o) const {return !(*this == o);}

    vect abs() const {vect r; for(usize i = 0; i < N; i++) r.data[i] = abs(this->data[i]); return r;}
    vect operator-() {
        if constexpr(is_simd) this->pack = _mm_sub_ps(_mm_set1_ps(0.0f), this->pack);
        else for(usize i = 0; i < N; i++) this->data[i] = -this->data[i]; return *this;}

    vect normalize() {
        static_assert(Is_Float<T>::value, "Not a float vec!");
        T l = len();
        if constexpr(is_simd) this->pack = _mm_div_ps(this->pack, _mm_set1_ps(l));
        else for(usize i = 0; i < N; i++) this->data[i] /= l;
        return *this;
    }
    vect normalized() const {
        static_assert(Is_Float<T>::value, "Not a float vec!");
        vect r;
        T l = len();
        if constexpr(is_simd) r.pack = _mm_div_ps(this->pack, _mm_set1_ps(l));
        else for(usize i = 0; i < N; i++) r.data[i] = this->data[i] / l;
        return r;
    }

    T len2() {
        return dot(*this, *this);
    }
    T len() {
        static_assert(Is_Float<T>::value, "Not a float vec!");
        return sqrt(len2());
    }

    const T* begin() const {
        return this->data;
    }
    const T* end() const {
        return this->data + N;
    }
    T* begin() {
        return this->data;
    }
    T* end() {
        return this->data + N;
    }
};

template<typename T, usize N> 
struct Type_Info<vect<T,N>> {
	static constexpr char name[] = "vect";
	static constexpr usize size = sizeof(vect<T,N>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
	using members = Type_List<Record_Field<T*,offset_of(&vect<T,N>::data),_data>>;
};

template<typename T, usize N>
vect<T,N> min(vect<T,N> x, vect<T,N> y) {
    if constexpr(vect<T,N>::is_simd) return {_mm_min_ps(x.pack, y.pack)};
    vect<T,N> r;
    for(usize i = 0; i < N; i++) r.data[i] = _MIN(x.data[i], y.data[i]);
    return r;
}

template<typename T, usize N>
vect<T,N> max(vect<T,N> x, vect<T,N> y) {
    if constexpr(vect<T,N>::is_simd) return {_mm_max_ps(x.pack, y.pack)};
    vect<T,N> r;
    for(usize i = 0; i < N; i++) r.data[i] = _MAX(x.data[i], y.data[i]);
    return r;
}

template<typename T, usize N>
T dot(vect<T,N> x, vect<T,N> y) {
    if constexpr(vect<T,N>::is_simd) return vect<f32,4>{_mm_dp_ps(x.pack, y.pack, 0xf1)}.x;
    T r = {};
    for(usize i = 0; i < N; i++) r += x.data[i] * y.data[i];
    return r;
}

template<typename T, usize N>
vect<T,N> lerp(vect<T,N> min, vect<T,N> max, T dist) {
    return min + (max - min) * dist;
}

template<typename T, usize N>
vect<T,N> clamp(vect<T,N> x, vect<T,N> min, vect<T,N> max) {
    return max(min(x, max), min);
}

template<typename T>
struct v2_base {
    union {
        struct{ T x, y; };
        T data[2];
    };
};
template<typename T>
struct v3_base {
    union {
        struct{ T x, y, z; };
        struct{ vect<T,2> xy; T z; };
        struct{ T x; vect<T,2> yz; };
        T data[3];
    };
};
template<typename T>
struct v4_base {
    union {
        struct{ T x, y, z, w; };
        struct{ vect<T,2> xy; vect<T,2> zw; };
        struct{ T x; vect<T,3> yz; T w; };
        struct{ vect<T,3> xyz; T w; };
        struct{ T x; vect<T,3> yzw; };
        T data[4];
        __m128 pack;
    };
};
template<typename T, usize N>
struct vN_base {
    T data[N];
};

using v2 = vect<f32, 2>;
using v3 = vect<f32, 3>;
using v4 = vect<f32, 4>;

using v2i = vect<i32, 2>;
using v3i = vect<i32, 3>;
using v4i = vect<i32, 4>;

inline v3 cross(v3 l, v3 r) {
	return {l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}
