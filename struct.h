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



#define MAKE_FIELDTYPE_I2(ftype, fieldName, index, classname) typedef ftype classname##_fieldType_##index;
#define MAKE_FIELDTYPE(tuple, classname) APPLY(MAKE_FIELDTYPE_I2, EXPAND3 tuple, classname)

#define MAKE_FIELD_I(fieldType, fieldName, index) fieldType fieldName;
#define MAKE_FIELD(tuple, extra) MAKE_FIELD_I tuple

#define MAKE_REFL_FIELD_I2(fieldType, fieldName, index, classname) \
{ \
	.offset = offsetof(classname##_t, fieldName), \
	.size = sizeof(fieldType), \
	.name = #fieldName, \
	/*.type = &fieldType##_type,*/ /*hmm, but types can be invalid names, ex: pointers*/ \
},
#define MAKE_REFL_FIELD(tuple, classname) APPLY(MAKE_REFL_FIELD_I2, EXPAND3 tuple, classname)

#define STRUCT(classname, ...) \
\
/* <class>_fieldType_<index>; */\
\
FOR_EACH(MAKE_FIELDTYPE, , classname, __VA_ARGS__) \
\
/* the class itself: */\
\
typedef struct classname##_s {\
FOR_EACH(MAKE_FIELD, , , __VA_ARGS__) \
} classname##_t; \
\
/* reflection fields */\
\
reflect_t classname##_fields[] = { \
FOR_EACH(MAKE_REFL_FIELD, , classname, __VA_ARGS__) \
};


// vtable.h?

/*
defines:

for each arg:
	typedef <returnType> (<class>_<func>_t)(<args>);
	<class>_<func>_t <class>_<func>;

STRUCT(<class>_vtable, ...)

<class>_vtable_t <class>_vtable = {
	.<func> = <class>_<func>,
	...
};

*/

#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I(funcName, returnType, funcArgs, classname) typedef returnType (classname##_##funcName##_t) funcArgs;
#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE(tuple, classname) APPLY(MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I, EXPAND3 tuple, classname)

#define MAKE_VTABLE_C_FUNC_PROTOTYPE_I(funcName, returnType, funcArgs, classname) classname##_##funcName##_t classname##_##funcName;
#define MAKE_VTABLE_C_FUNC_PROTOTYPE(tuple, classname) APPLY(MAKE_VTABLE_C_FUNC_PROTOTYPE_I, EXPAND3 tuple, classname)

//calling STRUCT from VTABLE:
//#define MAKE_VTABLE_STRUCT_FIELD_I(funcName, returnType, funcArgs, classname) ,(classname##_##funcName##_t*, funcName, 0)
//#define MAKE_VTABLE_STRUCT_FIELD(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELD_I, EXPAND3 tuple, classname)

//TODO I need to either add "index" as a 4th field to VTABLE(...) *OR* add it as a var to the FOR_EACH macro's callback
//#define MAKE_VTABLE_STRUCT_FIELDTYPE_I2(funcName, returnType, funcArgs, classname) typedef classname##_##funcName##_t * classname##_vtable_fieldType_##index;
//#define MAKE_VTABLE_STRUCT_FIELDTYPE(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELDTYPE_I2, EXPAND3 tuple, classname)

//manually expanding STRUCT into VTABLE
#define MAKE_VTABLE_STRUCT_FIELD_I(funcName, returnType, funcArgs, classname) classname##_##funcName##_t * funcName;
#define MAKE_VTABLE_STRUCT_FIELD(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELD_I, EXPAND3 tuple, classname)

#define MAKE_VTABLE_STRUCT_REFL_FIELD_I2(funcName, returnType, funcArgs, classname) \
{ \
	.offset = offsetof(classname##_vtable_t, funcName), \
	.size = sizeof(classname##_##funcName##_t), \
	.name = #funcName, \
	/*.type = &fieldType##_type,*/ /*hmm, but types can be invalid names, ex: pointers*/ \
},
#define MAKE_VTABLE_STRUCT_REFL_FIELD(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_REFL_FIELD_I2, EXPAND3 tuple, classname)

#define MAKE_VTABLE_OBJ_FIELD_I(funcName, returnType, funcArgs, classname) .funcName = classname##_##funcName,
#define MAKE_VTABLE_OBJ_FIELD(tuple, classname) APPLY(MAKE_VTABLE_OBJ_FIELD_I, EXPAND3 tuple, classname)

#define VTABLE(classname, ...) \
typedef struct classname##_s classname##_t; \
FOR_EACH(MAKE_VTABLE_MEMBER_C_FUNC_TYPE, , classname, __VA_ARGS__) \
FOR_EACH(MAKE_VTABLE_C_FUNC_PROTOTYPE, , classname, __VA_ARGS__) \
/* calling STRUCT from VTABLE -- having trouble: */ \
/*STRUCT(classname##_vtable*/ \
/*	FOR_EACH(MAKE_VTABLE_STRUCT_FIELD, , classname, __VA_ARGS__)*/ \
/*)*/ \
/* manually expanding it: */ \
/*FOR_EACH(MAKE_VTABLE_STRUCT_FIELDTYPE, , classname##_vtable, __VA_ARGS__) */ /* this one is having trouble without either a manual index *or* a FOR_EACH callback automatic index */ \
typedef struct classname##_vtable_s {\
FOR_EACH(MAKE_VTABLE_STRUCT_FIELD, , classname, __VA_ARGS__) \
} classname##_vtable_t; \
reflect_t classname##_vtable_fields[] = { \
FOR_EACH(MAKE_VTABLE_STRUCT_REFL_FIELD, , classname, __VA_ARGS__) \
}; \
/* end STRUCT call */ \
classname##_vtable_t classname##_vtable = { \
FOR_EACH(MAKE_VTABLE_OBJ_FIELD, , classname, __VA_ARGS__) \
};
