#pragma once

//all "object subclasses" should have these matching fields and methods

#define CLASS_object_fields ()
#define CLASS_object_methods (\
	(alloc, object_t *, ()),\
	(free, void, (object_t *)),\
	(destroy, void, (object_t *)),\
	(init, void, (object_t *))\
)
CLASS(object)

MAKE_DEFAULTS(object, ALLOC, FREE, DESTROY, INIT)


/*
so here's the alternative to using a GNU lambda for newobj
but it can't forward vararg functions (like string_fmt)
and casting doesn't work

this assumes the vtable has 'alloc', 'free', 'destroy', 'init' (with no args) first
... just like object_vtable_t does ...
and this assumes the vtable's created object from alloc has a 'v' vtable as its first field
*/
object_t * newobj_func(object_vtable_t * const vtable) {
	object_t * const newobjptr = vtable->alloc();
	newobjptr->v->init(newobjptr);
	return newobjptr;
}



//TODO a function + macro that calls
//but if this is a macro then "obj" gets re-evaluated ...
//and if this is a function then "func" needs extra qualifiers, and "obj" needs corret type casting
// (not to mention the problems of __VA_ARGS__)
// TODO store 'obj', but that means determining its type
// does C have a decltype() function?
#define CALL(obj, func, ...) \
obj->v->func(obj VA_ARGS(__VA_ARGS__))
