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

//must have same number of params as the tuple passed into the FOR_EACH
#define EXPAND3(a,b,c) a, b, c

#define APPLY(macro, ...) macro(__VA_ARGS__)


#define MAKE_FIELDTYPE_I2(ftype, fieldName, index, className) typedef ftype className##_fieldType_##index;
#define MAKE_FIELDTYPE(tuple, className) APPLY(MAKE_FIELDTYPE_I2, EXPAND3 tuple, className)

#define MAKE_FIELD_I(fieldType, fieldName, index) fieldType fieldName;
#define MAKE_FIELD(x, extra) MAKE_FIELD_I x

#define MAKE_REFL_FIELD_I2(fieldType, fieldName, index, className) { .offset=offsetof(className##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},
#define MAKE_REFL_FIELD(tuple, className) APPLY(MAKE_REFL_FIELD_I2, EXPAND3 tuple, className)


#define STRUCT(className, ...) \
FOR_EACH(MAKE_FIELDTYPE, EMPTY, className, __VA_ARGS__) \
typedef struct className##_s {\
FOR_EACH(MAKE_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
} className##_t; \
reflect_t className##_fields[] = { \
FOR_EACH(MAKE_REFL_FIELD, EMPTY, className, __VA_ARGS__) \
};
