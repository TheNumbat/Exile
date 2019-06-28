
#include <engine/dbg.h>
#include <engine/ds/map.h>
#include <engine/ds/map.inl>
#include <engine/ds/vector.inl>
#include <engine/util/reflect.h>

enum META_START_PARSING_HERE {};

// include all game files and stuff

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
};

struct f {
	ec yes = ec::two;
};

int func() {
	enum func_enum {
		yes
	};
	return 0;
}