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
#define MAKE_FIELDTYPE_I(className, ftype, fieldName, index)	typedef ftype className##_fieldType_##index;
#define MAKE_FIELDTYPE(x, className)	MAKE_FIELDTYPE_I x 
#endif

#if 0 //working on merging the extra arg into the tuple 
#define MAKE_FIELDTYPE_I2(className, ftype, fieldName, index, structType2)	typedef ftype className##_fieldType_##index;
#define MAKE_FIELDTYPE_I1(tuple)			MAKE_FIELDTYPE_I2(tuple)
#define MAKE_FIELDTYPE(tuple, className)	MAKE_FIELDTYPE_I1(EXPAND(tuple, (className)))
#endif

//# args must match tuple dim
#define MAKE_FIELD_I(className, fieldType, fieldName, index)	fieldType fieldName;
#define MAKE_FIELD(x, extra)			MAKE_FIELD_I x

#define MAKE_REFL_FIELD_I(className, fieldType, fieldName, index)	{ .offset=offsetof(className##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},
#define MAKE_REFL_FIELD(x, extra)			MAKE_REFL_FIELD_I x

#define STRUCT(className, ...) \
FOR_EACH(MAKE_FIELDTYPE, EMPTY, className, __VA_ARGS__) \
typedef struct className##_s {\
FOR_EACH(MAKE_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
} className##_t;\
reflect_t className##_fields[] = {\
FOR_EACH(MAKE_REFL_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
};




//second attempt at STRUCT
//but this uses the extra arg of FOR_EACH instead of a first arg of the tuple

//must have same number of params as the tuple passed into the FOR_EACH
#define EXPAND3(a,b,c) a, b, c


#define MAKE_FIELDTYPE2_I2(ftype, fieldName, index, className) typedef ftype className##_fieldType_##index;
#define MAKE_FIELDTYPE2_I(a,b) MAKE_FIELDTYPE2_I2(a,b)
#define MAKE_FIELDTYPE2(tuple, className)	MAKE_FIELDTYPE2_I(EXPAND3 tuple, clsasName)


#define MAKE_FIELD2_I(fieldType, fieldName, index) fieldType fieldName;
#define MAKE_FIELD2(x, extra) MAKE_FIELD2_I x

#define MAKE_REFL_FIELD2_I2(fieldType, fieldName, index, className) { .offset=offsetof(className##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},

//used to defer to expand the tuple
#define MAKE_REFL_FIELD2_I(a,b) MAKE_REFL_FIELD2_I2(a,b)

#define MAKE_REFL_FIELD2(tuple, className) MAKE_REFL_FIELD2_I(EXPAND3 tuple, className)

#define STRUCT2(className, ...) \
FOR_EACH(MAKE_FIELDTYPE2, EMPTY, className, __VA_ARGS__) \
typedef struct className##_s {\
FOR_EACH(MAKE_FIELD2, EMPTY, EMPTY, __VA_ARGS__) \
} className##_t; \
reflect_t className##_fields[] = { \
FOR_EACH(MAKE_REFL_FIELD2, EMPTY, className, __VA_ARGS__) \
};
