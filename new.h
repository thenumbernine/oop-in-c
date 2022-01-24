#pragma once


#include <stdlib.h>	//free


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

void delete(void * ptr) {
	if (ptr) free(ptr);
}
