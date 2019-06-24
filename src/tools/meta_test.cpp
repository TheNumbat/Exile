
#include <engine/dbg.h>
#include <engine/ds/map.h>
#include <engine/ds/map.inl>
#include <engine/ds/vector.inl>
#include <engine/util/reflect.h>

enum e;
struct s;

enum e {
	e_one,
	e_two
};

enum class ec {
	one,
	two
};

struct s {
	int i;
	unsigned int u;
	float f;
	double d;
};

int func() {
	enum func_enum {
		yes
	};
	return 0;
}
