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

//c++ equiv of "new type()"
//calls type_alloc and then calls type_init
#define DEFAULT_NEW(type)\
type##_t * type##_new() {\
	type##_t * obj = type##_alloc();\
	type##_init(obj);\
	return obj;\
}

// type_delete calls type_destroy and then type_free
// c++ equiv of "delete type"
#define DEFAULT_DELETE(type)\
void type##_delete(type##_t * const o) {\
	if (o) type##_destroy(o);\
	type##_free(o); /*_delete behavior:*/\
}


//make a list of defaults.
//usage: MAKE_DEFAULTS(type, DEFAULT1, DEFAULT2, ...)
//calls MAKE_DEFAULT1(type), MAKE_DEFAULT2(type),. ..
#define MAKE_DEFAULT(name, type) CONCAT(DEFAULT_,name)(type)
#define MAKE_DEFAULTS(type, ...)\
FOR_EACH(MAKE_DEFAULT, EMPTY, type, __VA_ARGS__)
