#pragma once

#include "new.h"	//new, delete
#include "macros.h"	//FOR_EACH, CONCAT


//class allocator -- for returning the  memory of the class
// c++ equiv of void * type::operator new(size_t)
#define DEFAULT_ALLOC(type)\
type##_t * type##_alloc() {\
	return new(type##_t);\
}

// c++ equiv of void type::operator delete(void *)
#define DEFAULT_FREE(type)\
void type##_free(type##_t * const obj) {\
	delete(obj);\
}

// c++ equiv of type::type()
#define DEFAULT_INIT(type)\
void type##_init(type##_t * const obj) {}

// c++ equiv of type::~type()
#define DEFAULT_DESTROY(type)\
void type##_destroy(type##_t * const obj) {}

// type_delete calls type_destroy and then type_free
// c++ equiv of "delete type"
#define DEFAULT_DELETE(type)\
void type##_delete(type##_t * const o) {\
	if (o) type##_destroy(o);\
	type##_free(o); /*_delete behavior:*/\
}


//TODO this reference "str_cat_move"
//so be sure to include "str.h" if you use this.
#define DEFAULT_TOSTR(type)\
str_t * type##_tostr(\
	type##_t const * const obj\
) {\
	str_t * s = str_new_c(#type);\
	if (!obj) {\
		return str_cat_move(s, str_new_c("NULL"));\
	}\
	s = str_cat_move(s, str_new("%p={", obj));\
	/* TODO HERE FOR_EACH over the reflect fields, and then call each member's _tostr() */\
	reflect_t * endOfFields = type##_fields + numberof(type##_fields);\
	for (reflect_t * field = type##_fields; field < endOfFields; ++field) {\
		if (field > type##_fields) {\
			s = str_cat_move(s, str_new_c(", "));\
		}\
		s = str_cat_move(s, str_new("%s=", field->name));\
/*		s = str_cat_move(s, tostring(  ));*/\
	}\
	s = str_cat_move(s, str_new_c("}"));\
	return s;\
}


//make a list of defaults.
//usage: MAKE_DEFAULTS(type, DEFAULT1, DEFAULT2, ...)
//calls MAKE_DEFAULT1(type), MAKE_DEFAULT2(type),. ..
#define MAKE_DEFAULT(name, type) CONCAT(DEFAULT_,name)(type)
#define MAKE_DEFAULTS(type, ...)\
FOR_EACH(MAKE_DEFAULT, EMPTY, type, __VA_ARGS__)



//c++ equiv of "new type()"
//calls type_alloc and then calls type_init
#define DEFAULT_NEW(type)\
type##_t * type##_new() {\
	type##_t * obj = type##_alloc();\
	type##_init(obj);\
	return obj;\
}


#define MAKE_NEW_FOR_INIT_ARGS(tuple, extra)	UNPACK2 tuple	
#define MAKE_NEW_FOR_INIT_CALL(tuple, extra)	ARG2_OF_2 tuple

//type_t * type_new(...) => calls type_alloc, type_init(...) 
//EMPTY doesn't work for 2nd suffix arg, gotta use , ,
#define MAKE_NEW_FOR_INIT(type, initSuffix, ...)\
type##_t * type##_new##initSuffix(\
FOR_EACH(MAKE_NEW_FOR_INIT_ARGS, COMMA, EMPTY, __VA_ARGS__)\
) {\
	type##_t * obj = type##_alloc();\
	type##_init##initSuffix(obj,\
FOR_EACH(MAKE_NEW_FOR_INIT_CALL, COMMA, EMPTY, __VA_ARGS__)\
	);\
	return obj;\
}

//DEFAULT_NEW(type) same as MAKE_NEW_FOR_INIT(type, )
