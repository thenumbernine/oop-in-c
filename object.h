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


//TODO a function + macro that calls
//but if this is a macro then "obj" gets re-evaluated ...
//and if this is a function then "func" needs extra qualifiers, and "obj" needs corret type casting
// (not to mention the problems of __VA_ARGS__)
// TODO store 'obj', but that means determining its type
// does C have a decltype() function?
#define CALL(obj, func, ...) \
obj->v->func(obj VA_ARGS(__VA_ARGS__))
