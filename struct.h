#pragma once

#include <stddef.h>	//offsetof
#include "macros.h"	//FOR_EACH


/*
type info: holds rtti, vtable, etc stuff
name = name of type/class
tostring = tostring method
*/
typedef struct type_s {
	char * name;
	size_t size;
} type_t;

#define MAKE_TYPEINFO(tname)\
type_t tname##_type = {.name = #tname, .size = sizeof(tname)};

typedef char * charp_t;
MAKE_TYPEINFO(charp_t)

typedef void * voidp_t;
MAKE_TYPEINFO(voidp_t);

MAKE_TYPEINFO(int)
MAKE_TYPEINFO(size_t)



//info for each field in a struct
typedef struct reflect_s {
	size_t offset;
	size_t size;
	char * name;
	type_t * type;	//pointer to the type info ... vtable? reflect? more?
} reflect_t;



//typedef <type> <className>_<fieldName>_fieldType;
#define MAKE_FIELDTYPE_I2(ftype, fieldName, classname) typedef ftype classname##_##fieldName##_fieldType;
#define MAKE_FIELDTYPE(tuple, classname) APPLY(MAKE_FIELDTYPE_I2, EXPAND2 tuple, classname)
#define MAKE_FIELDTYPES(classname, ...) FOR_EACH(MAKE_FIELDTYPE, , classname, __VA_ARGS__)

#define MAKE_STRUCT_FIELD_I(fieldType, fieldName) fieldType fieldName;
#define MAKE_STRUCT_FIELD(tuple, extra) MAKE_STRUCT_FIELD_I tuple

#define MAKE_STRUCT(classname, ...) \
typedef struct classname##_s {\
FOR_EACH(MAKE_STRUCT_FIELD, , , __VA_ARGS__) \
} classname##_t;


#define MAKE_REFLECT_FIELD_I(fieldType, fieldName, classname) \
{ \
	.offset = offsetof(classname##_t, fieldName), \
	.size = sizeof(fieldType), \
	.name = #fieldName, \
	.type = &fieldType##_type, /*hmm, but types can be invalid names, ex: pointers.  so, like the function pointers in the vtable, they will all have to be typedef'd.*/ \
},
#define MAKE_REFLECT_FIELD(tuple, classname) APPLY(MAKE_REFLECT_FIELD_I, EXPAND2 tuple, classname)

#define MAKE_REFLECT(classname, ...) \
reflect_t classname##_fields[] = { \
FOR_EACH(MAKE_REFLECT_FIELD, , classname, __VA_ARGS__) \
};


/*
... but this fails
because it depends on newobj
which depends on classname##_vtable
specifically, it needs vtable->init_##suffix
and this is non-standardized per-class
or I could just define string's vtable before defining this tostring in any function (including object)
or I could just make some kind of way to skip tostring in object?
*/
#if 0
typedef struct string_s string_t;
string_t * string_cat_move(string_t * a, string_t * b);
void string_init_c(string_t * s, char const * c);
void string_init_fmt(string_t * s, char const * fmt, ...);
#define MAKE_DEFAULT_TOSTRING_FIELD_I(fieldType, fieldName, classname) \
	s = string_cat_move(s, newobj(string,_c,", "));\
	s = string_cat_move(s, newobj(string,_fmt,"%s=", field->name));\
	s = string_cat_move(s, newobj(string,_c,"(value)")/*tostring(  )*/);
#define MAKE_DEFAULT_TOSTRING_FIELD(tuple, classname) APPLY(MAKE_DEFAULT_TOSTRING_FIELD_I, EXPAND2 tuple, classname)
#endif


#define MAKE_TYPE_AND_REFLECT(classname, ...) \
\
/* <class>_<field>_fieldType; */\
MAKE_FIELDTYPES(classname, __VA_ARGS__)\
\
/* the class itself: */\
MAKE_STRUCT(classname, __VA_ARGS__) \
\
/* reflection fields */\
MAKE_REFLECT(classname, __VA_ARGS__) \

#if 0
/* TODO while you're here, make the default _tostring function  */\
/*  because the _tostring needs the field types */\
/*  and right now field types aren't stored as rtti  */\
/*  so they need to be generated / accessed while we still have the fields */\
/*  which is here. */\
/* default tostring that auto-serializes fields */\
\
string_t * classname##_default_tostring(\
	classname##_t const * const obj\
) {\
	string_t * s = newobj(string,_c,#classname);\
	if (!obj) {\
		return string_cat_move(s, newobj(string,_c,"NULL"));\
	}\
	s = string_cat_move(s, newobj(string,_fmt,"%p={", obj));\
FOR_EACH(MAKE_DEFAULT_TOSTRING_FIELD, , classname, __VA_ARGS__) \
	s = string_cat_move(s, newobj(string,_c,"}"));\
	return s;\
}
#endif


// vtable.h?

/*
defines:

for each arg:
	typedef <returnType> (<class>_<func>_t)(<args>);
	<class>_<func>_t <class>_<func>;

MAKE_TYPE_AND_REFLECT(<class>_vtable, ...)

<class>_vtable_t <class>_vtable = {
	.<func> = <class>_<func>,
	...
};

*/

#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I(funcName, returnType, funcArgs, classname) typedef returnType (classname##_##funcName##_t) funcArgs;
#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE(tuple, classname) APPLY(MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I, EXPAND3 tuple, classname)

#define MAKE_VTABLE_C_FUNC_PROTOTYPE_I(funcName, returnType, funcArgs, classname) classname##_##funcName##_t classname##_##funcName;
#define MAKE_VTABLE_C_FUNC_PROTOTYPE(tuple, classname) APPLY(MAKE_VTABLE_C_FUNC_PROTOTYPE_I, EXPAND3 tuple, classname)

//calling MAKE_TYPE_AND_REFLECT from MAKE_VTABLE:
//#define MAKE_VTABLE_STRUCT_FIELD_I(funcName, returnType, funcArgs, classname) ,(classname##_##funcName##_t*, funcName, 0)
//#define MAKE_VTABLE_STRUCT_FIELD(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELD_I, EXPAND3 tuple, classname)

//#define MAKE_VTABLE_STRUCT_FIELDTYPE_I2(funcName, returnType, funcArgs, classname) typedef classname##_##funcName##_t * classname##_vtable_##funcName##_fieldType;
//#define MAKE_VTABLE_STRUCT_FIELDTYPE(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELDTYPE_I2, EXPAND3 tuple, classname)

//manually expanding MAKE_TYPE_AND_REFLECT into MAKE_VTABLE
#define MAKE_VTABLE_STRUCT_FIELD2_I(funcName, returnType, funcArgs, classname) classname##_##funcName##_t * funcName;
#define MAKE_VTABLE_STRUCT_FIELD2(tuple, classname) APPLY(MAKE_VTABLE_STRUCT_FIELD2_I, EXPAND3 tuple, classname)

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

#define MAKE_VTABLE(classname, ...) \
\
typedef struct classname##_s classname##_t; \
FOR_EACH(MAKE_VTABLE_MEMBER_C_FUNC_TYPE, , classname, __VA_ARGS__) \
FOR_EACH(MAKE_VTABLE_C_FUNC_PROTOTYPE, , classname, __VA_ARGS__) \
\
/* calling MAKE_TYPE_AND_REFLECT from MAKE_VTABLE -- having trouble: */ \
/*MAKE_TYPE_AND_REFLECT(classname##_vtable*/ \
/*	FOR_EACH(MAKE_VTABLE_STRUCT_FIELD, , classname VA_ARGS(__VA_ARGS__))*/ \
/*)*/ \
/* manually expanding it: */ \
/*FOR_EACH(MAKE_VTABLE_STRUCT_FIELDTYPE, , classname##_vtable, __VA_ARGS__) */ \
typedef struct classname##_vtable_s {\
FOR_EACH(MAKE_VTABLE_STRUCT_FIELD2, , classname, __VA_ARGS__) \
} classname##_vtable_t; \
MAKE_TYPEINFO(classname##_vtable_t)\
\
typedef classname##_vtable_t * classname##_vtable_p;\
MAKE_TYPEINFO(classname##_vtable_p)\
typedef classname##_vtable_t const * classname##_vtable_cp;\
MAKE_TYPEINFO(classname##_vtable_cp)\
\
reflect_t classname##_vtable_fields[] = { \
FOR_EACH(MAKE_VTABLE_STRUCT_REFL_FIELD, , classname, __VA_ARGS__) \
}; \
\
/* end MAKE_TYPE_AND_REFLECT call */ \
classname##_vtable_t classname##_vtable = { \
FOR_EACH(MAKE_VTABLE_OBJ_FIELD, , classname, __VA_ARGS__) \
};


//here's the main class generation
#define GENERATE_CLASS(className, structFields, vtableFields) \
\
/* make the vtable first */\
MAKE_VTABLE(className, DEFER vtableFields) \
\
/* make the struct next */\
MAKE_TYPE_AND_REFLECT( \
	className, \
	/* insert the vtable first: */\
	(className##_vtable_cp, v) \
	/* need this deferred for if structFields has zero args */\
	DEFER_VA_ARGS structFields \
)


/*
call the class generation
make sure you have the following defined:
	
	CLASS_<className>_fields = tuple of (type, name)
	
	CLASS_<className>_methods = tuple of (name, returnType, (args))
*/
#define CLASS(className) \
GENERATE_CLASS(className, CLASS_##className##_fields, CLASS_##className##_methods)
