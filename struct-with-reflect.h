#pragma once

#include <stddef.h>	//offsetof
#include "macros.h"	//FOR_EACH

typedef struct reflect_s {
	size_t offset;
	size_t size;
	char * name;
} reflect_t;


//FOR_EACH can forward ... but you gotta CONCAT args to eval them
//typedef <type> <name>_fieldType_<number>;
#if 1 // works
#define MAKE_FIELDTYPE_I(structType, ftype, fieldName, index)	typedef ftype structType##_fieldType_##index;
#define MAKE_FIELDTYPE(x, className)	MAKE_FIELDTYPE_I x 
#endif

#if 0 //working on merging the extra arg into the tuple 
#define MAKE_FIELDTYPE_I2(structType, ftype, fieldName, index, structType2)	typedef ftype structType##_fieldType_##index;
#define MAKE_FIELDTYPE_I1(tuple)			MAKE_FIELDTYPE_I2(tuple)
#define MAKE_FIELDTYPE(tuple, className)	MAKE_FIELDTYPE_I1(EXPAND(tuple, (className)))
#endif

//# args must match tuple dim
#define MAKE_FIELD_I(structType, fieldType, fieldName, index)	fieldType fieldName;
#define MAKE_FIELD(x, extra)			MAKE_FIELD_I x

#define MAKE_REFL_FIELD_I(structType, fieldType, fieldName, index)	{ .offset=offsetof(structType##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},
#define MAKE_REFL_FIELD(x, extra)			MAKE_REFL_FIELD_I x

#define STRUCT(className, ...) \
FOR_EACH(MAKE_FIELDTYPE, EMPTY, className, __VA_ARGS__) \
typedef struct className##_s {\
FOR_EACH(MAKE_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
} className##_t;\
reflect_t className##_fields[] = {\
FOR_EACH(MAKE_REFL_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
};
