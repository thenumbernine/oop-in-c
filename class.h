#pragma once

#include "new.h"	//new, deleteprim
#include "macros.h"	//FOR_EACH, CONCAT

/*
class_alloc <=> void * type::operator new(size_t)
class_free <=> class::operator delete(void *)
class_init <=> class::class()
class_destroy <=> class::~class()
obj = class_new() <=> obj = new class();		... calls _alloc, sets vtable, calls _init
delete obj <=> delete obj;	... calls _destroy and _free
class_tostr(obj) <=> to_string(obj);
*/

//class allocator -- for returning the  memory of the class
// c++ equiv of void * type::operator new(size_t)
#define DEFAULT_ALLOC(type)\
type##_t * type##_alloc() {\
	return new(type##_t);\
}

// c++ equiv of void type::operator delete(void *)
#define DEFAULT_FREE(type)\
void type##_free(type##_t * const obj) {\
	deleteprim(obj);\
}

// c++ equiv of type::type()
#define DEFAULT_INIT(type)\
void type##_init(type##_t * const obj) {}

// c++ equiv of type::~type()
#define DEFAULT_DESTROY(type)\
void type##_destroy(type##_t * const obj) {}

//c++ equiv of "new type()"
//type_t * type_new(...) => calls type_alloc, type_init(...) 
// C lambda is GNU-specific
#define newobj(type, suffix, ...) ({\
	type##_vtable_t const * const v = &type##_vtable;\
	type##_t * obj = v->alloc();\
	obj->v = v;\
	v->init##suffix(obj VA_ARGS(__VA_ARGS__));\
	obj;\
})

#if 0
#define newobj3(type, vtable, init, ...)	TODO
#define newobj2(type, vtable, suffix, ...)	newobj3(type, vtable, vtable->init##suffix, __VA_ARGS__)
#define newobj(type, suffix, ...)			newobj2(type, &type##_vtable, suffix, __VA_ARGS__)
#endif


// delete(obj) calls obj's destroy and then obj's free
// c++ equiv of "delete type"
#define deleteobj(o) {\
	if (o) {\
		o->v->destroy(o);\
		o->v->free(o);\
	}\
}


//TODO this reference "str_cat_move"
//so be sure to include "str.h" if you use this.
#define DEFAULT_TOSTR(type)\
str_t * type##_tostr(\
	type##_t const * const obj\
) {\
	str_t * s = newobj(str,_c,#type);\
	if (!obj) {\
		return str_cat_move(s, newobj(str,_c,"NULL"));\
	}\
	s = str_cat_move(s, newobj(str,_fmt,"%p={", obj));\
	/* TODO HERE FOR_EACH over the reflect fields, and then call each member's _tostr() */\
	reflect_t const * const endOfFields = type##_fields + numberof(type##_fields);\
	for (reflect_t const * field = type##_fields; field < endOfFields; ++field) {\
		if (field > type##_fields) {\
			s = str_cat_move(s, newobj(str,_c,", "));\
		}\
		s = str_cat_move(s, newobj(str,_fmt,"%s=", field->name));\
/*		s = str_cat_move(s, tostring(  ));*/\
	}\
	s = str_cat_move(s, newobj(str,_c,"}"));\
	return s;\
}


//make a list of defaults.
//usage: MAKE_DEFAULTS(type, DEFAULT1, DEFAULT2, ...)
//calls MAKE_DEFAULT1(type), MAKE_DEFAULT2(type),. ..
#define MAKE_DEFAULT(name, type) CONCAT(DEFAULT_,name)(type)
#define MAKE_DEFAULTS(type, ...)\
FOR_EACH(MAKE_DEFAULT, , type, __VA_ARGS__)
