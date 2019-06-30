
#if 1
#include <engine/dbg.h>
#include <engine/ds/map.h>
#include <engine/ds/map.inl>
#include <engine/ds/vector.inl>
#include <engine/util/reflect.h>

enum e : unsigned char {
	e_three = 2,
	e_one = 1,
	e_two = 1
};

enum class ec : uint32_t {
	one,
	two
};

struct f;

struct NOREFLECT hidden {};

struct s {
	int i;
	unsigned int u;
	float f;
	double d;

	struct f* eff;

	enum {
		OMEGAUL
	};

	enum class e_in_s : i64 {
		head
	};

	void some_function() {}
	void some_other_function();
	int typed_function();

	struct {
		int int_in_anon;
	};

	e_in_s LULw;

	e_in_s e_array[5];

	bool (*test)(int i) = null;
};

struct f {
	ec yes = ec::two;
};

template<typename E>
struct EEEE {
	void func(bool (*yes)(E)) {}
};
#endif

struct stackoverflow0;

struct stackoverflow1 {
	stackoverflow0* next;
};
struct stackoverflow0 {
	stackoverflow1* next;
};

void func() {
	stackoverflow0 LOL;
}
