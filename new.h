#pragma once


#include <stdlib.h>	//free


#define newprim(x)		(x*)safealloc(sizeof(x))

// TODO 'newprimarray'
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

//technically not the "delete" operator
// because it doesn't call the dtor
void deleteprim(void * ptr) {
	if (ptr) free(ptr);
}
