#pragma once

#include <stdio.h>	//fprintf
#include <stdlib.h>	//exit

void fail_cstr(char const * const s) {
	fprintf(stderr, "%s\n", s);
	exit(1);
}

//this leaks the allocated string, but then it calls exit(1) so ...
#define fail(...)	(\
	fail_cstr(\
		newobj(str, _fmt, __VA_ARGS__)->ptr\
	)\
)
