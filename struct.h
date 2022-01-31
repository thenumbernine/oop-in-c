#pragma once

#include <stddef.h>	//offsetof
#include "macros.h"	//FOR_EACH

//.tostring needs this:
typedef struct string_s string_t;

/*
type info: holds rtti, vtable, etc stuff
name = name of type/class
tostring = tostring method
*/
typedef struct type_s {
	char * name;
	size_t size;
	string_t * (*tostring)(/*type_t*/void const * obj);		//I don't like the idea of casting, but here it has to be
} typeinfo_t;

#define MAKE_TYPEINFO(tname)\
typeinfo_t tname##_type = {\
	.name = #tname,\
	.size = sizeof(tname),\
	.tostring = &tname##_tostring,\
};

#define MAKE_TYPEINFO_WITH_TOSTRING(tname, tostringFunc)\
typeinfo_t tname##_type = {\
	.name = #tname,\
	.size = sizeof(tname),\
	.tostring = &tostringFunc,\
};

// you can't use this until after string.h is included
// TODO maybe this is ref instead of ptr
// since it returns the obj addr, not the contents of a pointer 
#define MAKE_TOSTRING_FOR_ADDR(name)\
string_t * name##_tostring(void const * obj) {\
	return newobj(string,_fmt,#name "(%p)", obj);\
}

//NOTICE bodies come at the end of string.h
string_t * charp_t_tostring(void const * obj);
string_t * voidp_t_tostring(void const * obj);
string_t * int_tostring(void const * obj);
string_t * size_t_tostring(void const * obj);

typedef char * charp_t;
MAKE_TYPEINFO(charp_t)

typedef void * voidp_t;
MAKE_TYPEINFO(voidp_t);

MAKE_TYPEINFO(int)
MAKE_TYPEINFO(size_t)


//used by all vtables for now, so I don't have to worry about vtable tostring generation yet 
string_t * vtable_tostring(void const * obj);
string_t * func_tostring(void const * obj);


//info for each field in a struct
typedef struct reflect_s {
	size_t offset;
	char * name;
	typeinfo_t * type;	//pointer to the type info ... vtable? reflect? more?
} reflect_t;



//typedef <type> <className>_<fieldName>_fieldType;
#define MAKE_FIELDTYPE_I2(ftype, fieldName, className) typedef ftype className##_##fieldName##_fieldType;
#define MAKE_FIELDTYPE(tuple, className) APPLY(MAKE_FIELDTYPE_I2, EXPAND2 tuple, className)
#define MAKE_FIELDTYPES(className, ...) FOR_EACH(MAKE_FIELDTYPE, , className, __VA_ARGS__)

#define MAKE_STRUCT_FIELD(tuple, extra) UNPACK2 tuple;

#define MAKE_STRUCT(className, ...) \
typedef struct className##_s {\
FOR_EACH(MAKE_STRUCT_FIELD, , , __VA_ARGS__) \
} className##_t;

#define MAKE_REFLECT_FIELD_ENTRY_I(fieldType, fieldName, className) \
reflect_t className##_##fieldName##_field = { \
	.offset = offsetof(className##_t, fieldName), \
	.name = #fieldName, \
	.type = &fieldType##_type, /*hmm, but types can be invalid names, ex: pointers.  so, like the function pointers in the vtable, they will all have to be typedef'd.*/ \
};
#define MAKE_REFLECT_FIELD_ENTRY(tuple, className) APPLY(MAKE_REFLECT_FIELD_ENTRY_I, EXPAND2 tuple, className)

#define MAKE_REFLECT_STRUCT_ENTRY_I(fieldType, fieldName, className) &className##_##fieldName##_field
#define MAKE_REFLECT_STRUCT_ENTRY(tuple, className) APPLY(MAKE_REFLECT_STRUCT_ENTRY_I, EXPAND2 tuple, className)

#define MAKE_REFLECT(className, ...) \
\
FOR_EACH(MAKE_REFLECT_FIELD_ENTRY, , className, __VA_ARGS__) \
\
reflect_t const * className##_fields[] = { \
FOR_EACH(MAKE_REFLECT_STRUCT_ENTRY, COMMA, className, __VA_ARGS__), \
};


/*
... but this fails
because it depends on newobj
which depends on className##_vtable
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
#define MAKE_DEFAULT_TOSTRING_FIELD_I(fieldType, fieldName, className) \
	s = string_cat_move(s, newobj(string,_c,", "));\
	s = string_cat_move(s, newobj(string,_fmt,"%s=", field->name));\
	s = string_cat_move(s, newobj(string,_c,"(value)")/*tostring(  )*/);
#define MAKE_DEFAULT_TOSTRING_FIELD(tuple, className) APPLY(MAKE_DEFAULT_TOSTRING_FIELD_I, EXPAND2 tuple, className)
#endif


#define MAKE_TYPE_AND_REFLECT(className, ...) \
\
/* <class>_<field>_fieldType; */\
MAKE_FIELDTYPES(className, __VA_ARGS__)\
\
/* the class itself: */\
MAKE_STRUCT(className, __VA_ARGS__) \
\
/* reflection fields */\
MAKE_REFLECT(className, __VA_ARGS__) \

#if 0
/* TODO while you're here, make the default _tostring function  */\
/*  because the _tostring needs the field types */\
/*  and right now field types aren't stored as rtti  */\
/*  so they need to be generated / accessed while we still have the fields */\
/*  which is here. */\
/* default tostring that auto-serializes fields */\
\
string_t * className##_default_tostring(\
	className##_t const * const obj\
) {\
	string_t * s = newobj(string,_c,#className);\
	if (!obj) {\
		return string_cat_move(s, newobj(string,_c,"NULL"));\
	}\
	s = string_cat_move(s, newobj(string,_fmt,"%p={", obj));\
FOR_EACH(MAKE_DEFAULT_TOSTRING_FIELD, , className, __VA_ARGS__) \
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

#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I(funcName, returnType, funcArgs, className) \
typedef returnType (className##_##funcName##_t) funcArgs; \
MAKE_TYPEINFO_WITH_TOSTRING(className##_##funcName##_t, func_tostring)

#define MAKE_VTABLE_MEMBER_C_FUNC_TYPE(tuple, className) APPLY(MAKE_VTABLE_MEMBER_C_FUNC_TYPE_I, EXPAND3 tuple, className)

#define MAKE_VTABLE_C_FUNC_PROTOTYPE_I(funcName, returnType, funcArgs, className) className##_##funcName##_t className##_##funcName;
#define MAKE_VTABLE_C_FUNC_PROTOTYPE(tuple, className) APPLY(MAKE_VTABLE_C_FUNC_PROTOTYPE_I, EXPAND3 tuple, className)

//calling MAKE_TYPE_AND_REFLECT from MAKE_VTABLE:
//#define MAKE_VTABLE_STRUCT_FIELD_I(funcName, returnType, funcArgs, className) ,(className##_##funcName##_t*, funcName, 0)
//#define MAKE_VTABLE_STRUCT_FIELD(tuple, className) APPLY(MAKE_VTABLE_STRUCT_FIELD_I, EXPAND3 tuple, className)

//#define MAKE_VTABLE_STRUCT_FIELDTYPE_I2(funcName, returnType, funcArgs, className) typedef className##_##funcName##_t * className##_vtable_##funcName##_fieldType;
//#define MAKE_VTABLE_STRUCT_FIELDTYPE(tuple, className) APPLY(MAKE_VTABLE_STRUCT_FIELDTYPE_I2, EXPAND3 tuple, className)

//manually expanding MAKE_TYPE_AND_REFLECT into MAKE_VTABLE
#define MAKE_VTABLE_STRUCT_FIELD2_I(funcName, returnType, funcArgs, className) className##_##funcName##_t * funcName;
#define MAKE_VTABLE_STRUCT_FIELD2(tuple, className) APPLY(MAKE_VTABLE_STRUCT_FIELD2_I, EXPAND3 tuple, className)

#define MAKE_VTABLE_REFLECT_FIELD_ENTRY_I(funcName, returnType, funcArgs, className) \
reflect_t className##_vtable_t_##funcName##_field = { \
	.offset = offsetof(className##_vtable_t, funcName), \
	.name = #funcName, \
	.type = &className##_##funcName##_t_type, /*hmm, but types can be invalid names, ex: pointers*/ \
};
#define MAKE_VTABLE_REFLECT_FIELD_ENTRY(tuple, className) APPLY(MAKE_VTABLE_REFLECT_FIELD_ENTRY_I, EXPAND3 tuple, className)

#define MAKE_VTABLE_REFLECT_STRUCT_ENTRY_I(funcName, returnType, funcArgs, className) &className##_vtable_t_##funcName##_field
#define MAKE_VTABLE_REFLECT_STRUCT_ENTRY(tuple, className) APPLY(MAKE_VTABLE_REFLECT_STRUCT_ENTRY_I, EXPAND3 tuple, className)

#define MAKE_VTABLE_OBJ_FIELD_I(funcName, returnType, funcArgs, className) .funcName = className##_##funcName,
#define MAKE_VTABLE_OBJ_FIELD(tuple, className) APPLY(MAKE_VTABLE_OBJ_FIELD_I, EXPAND3 tuple, className)

#define MAKE_VTABLE(className, ...) \
\
typedef struct className##_s className##_t; \
FOR_EACH(MAKE_VTABLE_MEMBER_C_FUNC_TYPE, , className, __VA_ARGS__) \
FOR_EACH(MAKE_VTABLE_C_FUNC_PROTOTYPE, , className, __VA_ARGS__) \
\
/* calling MAKE_TYPE_AND_REFLECT from MAKE_VTABLE -- having trouble: */ \
/*MAKE_TYPE_AND_REFLECT(className##_vtable*/ \
/*	FOR_EACH(MAKE_VTABLE_STRUCT_FIELD, , className VA_ARGS(__VA_ARGS__))*/ \
/*)*/ \
/* manually expanding it: */ \
/*FOR_EACH(MAKE_VTABLE_STRUCT_FIELDTYPE, , className##_vtable, __VA_ARGS__) */ \
typedef struct className##_vtable_s {\
FOR_EACH(MAKE_VTABLE_STRUCT_FIELD2, , className, __VA_ARGS__) \
} className##_vtable_t; \
\
MAKE_TYPEINFO_WITH_TOSTRING(className##_vtable_t, vtable_tostring)\
\
typedef className##_vtable_t * className##_vtable_p;\
MAKE_TYPEINFO_WITH_TOSTRING(className##_vtable_p, vtable_tostring)\
\
typedef className##_vtable_t const * className##_vtable_cp;\
MAKE_TYPEINFO_WITH_TOSTRING(className##_vtable_cp, vtable_tostring)\
\
FOR_EACH(MAKE_VTABLE_REFLECT_FIELD_ENTRY, , className, __VA_ARGS__) \
\
reflect_t const * className##_vtable_fields[] = { \
FOR_EACH(MAKE_VTABLE_REFLECT_STRUCT_ENTRY, COMMA, className, __VA_ARGS__), \
}; \
\
/* end MAKE_TYPE_AND_REFLECT call */ \
className##_vtable_t className##_vtable = { \
FOR_EACH(MAKE_VTABLE_OBJ_FIELD, , className, __VA_ARGS__) \
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
