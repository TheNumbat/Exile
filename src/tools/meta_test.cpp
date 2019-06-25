

#include <engine/dbg.h>
#include <engine/ds/map.h>
#include <engine/ds/map.inl>
#include <engine/ds/vector.inl>
#include <engine/util/reflect.h>

#include "engine/basic.h"

enum e : unsigned char {
	e_three = 2,
	e_one = 1,
	e_two = 1
};

enum class ec : uint32_t {
	one,
	two
};

struct s {
	int i;
	unsigned int u;
	float f;
	double d;

	enum class e_in_s : i64 {
		head
	};
};

int func() {
	enum func_enum {
		yes
	};
	return 0;
}
