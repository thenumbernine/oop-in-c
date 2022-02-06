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

#define FOR_EACH_1(what, between, extra, x, ...) what(x, extra)
#define FOR_EACH_2(what, between, extra, x, ...) what(x, extra) between FOR_EACH_1(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_3(what, between, extra, x, ...) what(x, extra) between FOR_EACH_2(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_4(what, between, extra, x, ...) what(x, extra) between FOR_EACH_3(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_5(what, between, extra, x, ...) what(x, extra) between FOR_EACH_4(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_6(what, between, extra, x, ...) what(x, extra) between FOR_EACH_5(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_7(what, between, extra, x, ...) what(x, extra) between FOR_EACH_6(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_8(what, between, extra, x, ...) what(x, extra) between FOR_EACH_7(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_9(what, between, extra, x, ...) what(x, extra) between FOR_EACH_8(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_10(what, between, extra, x, ...) what(x, extra) between FOR_EACH_9(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_11(what, between, extra, x, ...) what(x, extra) between FOR_EACH_10(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, N, ...) N
#define FOR_EACH_RSEQ_N() 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define FOR_EACH_(N, what, between, extra, ...) CONCAT(FOR_EACH_, N)(what, DEFER(between), DEFER(extra), __VA_ARGS__)
#define FOR_EACH(what, between, extra, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, DEFER(between), DEFER(extra), __VA_ARGS__)


//used to unpack the ()'s
// you can't just call DEFER in the FOR_EACH, you gotta do this
#define EXPAND_I(x, extra)			DEFER x

// converts macro tuple args into a single expanded list:
// (a, b), (c, d, e), (f) => a, b, c, d, e, f
#define EXPAND(...)\
FOR_EACH(EXPAND_I, COMMA, , __VA_ARGS__)


#define numberof(x)		(sizeof(x)/sizeof(*(x)))
#define endof(x)		((x) + numberof(x))
