#pragma once


#include <stdlib.h>	//free

//TODO separate new for heap / safealloc (malloc) from new for stack / alloca
#define newprim(x)		(x*)safealloc(sizeof(x))

// TODO 'newprimarray'
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

//technically not the "delete" operator
// because it doesn't call the dtor
#define deleteprim free
