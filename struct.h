#pragma once

#include <stddef.h>	//offsetof
#include "macros.h"	//FOR_EACH


#if 0
/*
type info: holds rtti, vtable, etc stuff
name = name of type/class
tostring = tostring method
*/
typedef struct type_s {
	char * name;
} type_t;


type_t size_t_type = {.name = "size_t"};
#endif


//info for each field in a struct
typedef struct reflect_s {
	size_t offset;
	size_t size;
	char * name;
//	type_t * type;	//pointer to the type info ... vtable? reflect? more?
} reflect_t;



//FOR_EACH can forward ... but you gotta CONCAT args to eval them
//typedef <type> <name>_fieldType_<number>;



#define MAKE_FIELDTYPE_I2(ftype, fieldName, index, classname) \
typedef ftype classname##_fieldType_##index;

#define MAKE_FIELDTYPE(tuple, classname) \
APPLY(MAKE_FIELDTYPE_I2, EXPAND3 tuple, classname)

#define MAKE_FIELD_I(fieldType, fieldName, index) fieldType fieldName;

#define MAKE_FIELD(tuple, extra) MAKE_FIELD_I tuple

#define MAKE_REFL_FIELD_I2(fieldType, fieldName, index, classname) \
{ \
	.offset = offsetof(classname##_t, fieldName), \
	.size = sizeof(fieldType), \
	.name = #fieldName, \
	/*.type = &fieldType##_type,*/ /*hmm, but types can be invalid names, ex: pointers*/ \
},

#define MAKE_REFL_FIELD(tuple, classname) \
APPLY(MAKE_REFL_FIELD_I2, EXPAND3 tuple, classname)


#define STRUCT(classname, ...) \
\
/* <class>_fieldType_<index>; */\
\
FOR_EACH(MAKE_FIELDTYPE, EMPTY, classname, __VA_ARGS__) \
\
/* the class itself: */\
\
typedef struct classname##_s {\
FOR_EACH(MAKE_FIELD, EMPTY, EMPTY, __VA_ARGS__) \
} classname##_t; \
\
/* reflection fields */\
\
reflect_t classname##_fields[] = { \
FOR_EACH(MAKE_REFL_FIELD, EMPTY, classname, __VA_ARGS__) \
};


// vtable.h?


//#define VTABLE(classname
