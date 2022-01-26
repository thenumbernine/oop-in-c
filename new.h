#pragma once


#include <stdlib.h>	//free


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

//technically not the "delete" operator
// because it doesn't call the dtor
void deleteprim(void * ptr) {
	if (ptr) free(ptr);
}
