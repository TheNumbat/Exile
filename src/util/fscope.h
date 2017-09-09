
#pragma once

#ifdef DO_PROF
struct func_scope {
	func_scope(code_context context);
	~func_scope();
};

struct func_scope_nocs {
	func_scope_nocs(code_context context);
	~func_scope_nocs();
};
#define PROF func_scope __f(CONTEXT);
#define PROF_NOCS func_scope_nocs __f(CONTEXT);
#else
#define PROF 
#define PROF_NOCS
#endif
