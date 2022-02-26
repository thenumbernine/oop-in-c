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
class_tostring(obj) <=> to_string(obj);
*/

//class allocator -- for returning the  memory of the class
// c++ equiv of void * type::operator new(size_t)
#define DEFAULT_ALLOC(type)\
type##_t * type##_alloc() {\
	return newprim(type##_t);\
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
//type_t * newobj(suffix, ...) => calls type_alloc, type_init_suffix(...) 
//can't forward va-args in C.  says in: https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
// so to get around this, I'm using C lambda GCC specific trick:
// NOTICE using the var with matching name of a parent scope var will mess up the parent scope var
// another NOTICE ... 'newobj' using GNU lambda extension does not work in global scope!
#define newobj(type, suffix, ...) ({\
	type##_t * const newobjptr = type##_vtable.alloc();\
	newobjptr->v = &type##_vtable;\
	type##_vtable.init##suffix(newobjptr VA_ARGS(__VA_ARGS__));\
	newobjptr;\
})

//c++ equiv of stack-allocation (I think)
// this will depend on gcc extension lambdas like 'newobj' does
// it will also use the builtin stack allocator, so I guess :new is bypassed, which I suppose is the stack object behavior in C++ as well, right?
// This is especially for use in conjunction with THROW, but can be used elsewhere (why not?)
#define newstack(type, suffix, ...) ({\
	type##_t * const newobjptr = (type##_t *)alloca(sizeof(type##_t));\
	newobjptr->v = &type##_vtable;\
	type##_vtable.init##suffix(newobjptr VA_ARGS(__VA_ARGS__));\
	newobjptr;\
})

//TODO is there some way to implement "newobj", *with* vararg forwarding,
// and still not depend on GCC specific lambdas?
#if 0
#define newobj3(type, vtable, init, ...)	TODO
#define newobj2(type, vtable, suffix, ...)	newobj3(type, vtable, (vtable)->init##suffix, __VA_ARGS__)
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


//TODO this reference "string_cat_move"
//so be sure to include "string.h" if you use this.
#define DEFAULT_TOSTRING(className)\
string_t * className##_tostring(\
	void const * const objv\
) {\
	string_t * s = newobj(string,_c,#className);\
	if (!objv) {\
		return string_cat_move(s, newobj(string,_c,"NULL"));\
	}\
	s = string_cat_move(s, newobj(string,_fmt,"(0x%p)={", objv));\
	reflect_t const ** const endOfFields = className##_fields + numberof(className##_fields);\
	for (reflect_t const ** field = className##_fields; field < endOfFields; ++field) {\
		if (field > className##_fields) {\
			s = string_cat_move(s, newobj(string,_c,", "));\
		}\
		s = string_cat_move(s, newobj(string,_fmt,"%s=", (*field)->name));\
		s = string_cat_move(s, (*field)->type->tostring((void const *)((char const*)objv + (*field)->offset)));\
	}\
	s = string_cat_move(s, newobj(string,_c,"}"));\
	return s;\
}


//make a list of defaults.
//usage: MAKE_DEFAULTS(type, DEFAULT1, DEFAULT2, ...)
//calls MAKE_DEFAULT1(type), MAKE_DEFAULT2(type),. ..
#define MAKE_DEFAULT(name, type) CONCAT(DEFAULT_,name)(type)
#define MAKE_DEFAULTS(type, ...)\
FOR_EACH(MAKE_DEFAULT, , type, __VA_ARGS__)
