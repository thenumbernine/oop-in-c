#pragma once


#define COMMA ,		//for when a macro argument needs to be a comma

//technically is this an "unpack" since it removes the commas?
#define DEFER(...)				__VA_ARGS__

#define UNPACK2(a, b)	a b

#define ARG2_OF_2(a, b)	b

#define EXPAND2(a,b)   a, b
#define EXPAND3(a,b,c) a, b, c

#define APPLY(macro, ...) macro(__VA_ARGS__)

//trick for forwarding varargs that can be optionally zero sized
//https://stackoverflow.com/questions/5891221/variadic-macros-with-zero-arguments
#define VA_ARGS(...) , ##__VA_ARGS__

#define DEFER_VA_ARGS(...) VA_ARGS(__VA_ARGS__)


#define CONCAT(arg1, arg2) CONCAT1(arg1, arg2)
#define CONCAT1(arg1, arg2) CONCAT2(arg1, arg2)
#define CONCAT2(arg1, arg2) arg1##arg2


//https://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
// but in FOR_EACH replace "x, ..." with "..." and "x, __VA_ARGS__" with "__VA_ARGS__"
//https://stackoverflow.com/questions/65997123/generalized-iteration-over-arguments-of-macro-in-the-c-preprocessor
// looks like you need the extra DEFER(between) for when "between" is set to an empty arg
// otherwise the empty string will screw up the number of args / argument order
// "extra" is an extra parameter/tuple for the for_each scope that you might want all iterators to see

// TODO can I automate this with macros and #includes?
// can I have expressions in the #define name? #define (FOR_EACH_##COUNTER) (...) ?
// until then, here's my script-generated header:
#include "foreach.h"

//used to unpack the ()'s
// you can't just call DEFER in the FOR_EACH, you gotta do this
#define EXPAND_I(x, extra)			DEFER x

// converts macro tuple args into a single expanded list:
// (a, b), (c, d, e), (f) => a, b, c, d, e, f
#define EXPAND(...)\
FOR_EACH(EXPAND_I, COMMA, , __VA_ARGS__)


#define numberof(x)		(sizeof(x)/sizeof(*(x)))
#define endof(x)		((x) + numberof(x))
#define MIN(a,b)		((a) < (b) ? (a) : (b))
