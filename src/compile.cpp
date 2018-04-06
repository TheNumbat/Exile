
/* debug options

#define BOUNDS_CHECK 			// check access on array_get/vector_get
	
#define BLOCK_OR_EXIT_ON_ERROR	// __debugbreak in debugger or exit() on log_level::error | log_level::fatal always does this
	
#define PROFILE					// do function enter/exit profiling
	
#define MORE_PROF				// do profiling for functions that are called a _lot_
								// (still excludes vec constructors)

#define ZERO_ARENA				// memset arena allocator store to zero on reset

#define NO_CONCURRENT_JOBS		// makes queue_job just run the job and wait_job do nothing
*/

#define RELEASE // turn off everything for a true release build

#include "engine/compile.h"
engine* eng = null;

#include "world.h"
#include "exile.h"

#include "world.cpp"
#include "exile.cpp"
#include <meta_types.cpp>
