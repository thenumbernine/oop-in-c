#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>	//offsetof
#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <pthread.h>


///macros.h


#define EMPTY		//for when a macro argument needs to be empty
#define COMMA ,		//for when a macro argument needs to be a comma
#define DEFER(...)				__VA_ARGS__

#define CONCAT(arg1, arg2) CONCAT1(arg1, arg2)
#define CONCAT1(arg1, arg2) CONCAT2(arg1, arg2)
#define CONCAT2(arg1, arg2) arg1##arg2


//https://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
// but in FOR_EACH replace "x, ..." with "..." and "x, __VA_ARGS__" with "__VA_ARGS__"
//https://stackoverflow.com/questions/65997123/generalized-iteration-over-arguments-of-macro-in-the-c-preprocessor
// looks like you need the extra DEFER(between) for when "between" is set to EMPTY
// otherwise the empty string will screw up the number of args / argument order

#define FOR_EACH_1(what, between, x, ...) what(x)
#define FOR_EACH_2(what, between, x, ...) what(x) between FOR_EACH_1(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_3(what, between, x, ...) what(x) between FOR_EACH_2(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_4(what, between, x, ...) what(x) between FOR_EACH_3(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_5(what, between, x, ...) what(x) between FOR_EACH_4(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_6(what, between, x, ...) what(x) between FOR_EACH_5(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_7(what, between, x, ...) what(x) between FOR_EACH_6(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_8(what, between, x, ...) what(x) between FOR_EACH_7(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define FOR_EACH_(N, what, between, ...) CONCAT(FOR_EACH_, N)(what, DEFER(between), __VA_ARGS__)
#define FOR_EACH(what, between, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, DEFER(between), __VA_ARGS__)


#define numberof(x)		(sizeof(x)/sizeof(*(x)))
#define endof(x)		((x) + numberof(x))


//new.h


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

void delete(void * ptr) {
	if (ptr) free(ptr);
}


//struct-with-reflect.h


typedef struct reflect_s {
	size_t offset;
	size_t size;
	char * name;
} reflect_t;


//FOR_EACH can forward ... but you gotta CONCAT args to eval them
//typedef <type> <name>_fieldType_<number>;
#define MAKE_FIELDTYPE_I(structType, ftype, fieldName, index)	typedef ftype structType##_fieldType_##index;
#define MAKE_FIELDTYPE(x)	MAKE_FIELDTYPE_I x

//# args must match tuple dim
#define MAKE_FIELD_I(structType, fieldType, fieldName, index)	fieldType fieldName;
#define MAKE_FIELD(x)			MAKE_FIELD_I x

#define MAKE_REFL_FIELD_I(structType, fieldType, fieldName, index)	{ .offset=offsetof(structType##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},
#define MAKE_REFL_FIELD(x)			MAKE_REFL_FIELD_I x

#define STRUCT(type, ...) \
FOR_EACH(MAKE_FIELDTYPE, EMPTY, __VA_ARGS__) \
typedef struct type##_s {\
FOR_EACH(MAKE_FIELD, EMPTY, __VA_ARGS__) \
} type##_t;\
reflect_t type##_fields[] = {\
FOR_EACH(MAKE_REFL_FIELD, EMPTY, __VA_ARGS__) \
};


//class.h

#define DEFAULT_INIT(type)\
void type##_init(type##_t * const obj) {}

#define DEFAULT_DESTROY(type)\
void type##_destroy(type##_t * const obj) {}

//class allocator -- for returning the  memory of the class
// c++ equiv of void * ::operator new(size_t)
#define DEFAULT_ALLOC(type)\
type##_t * type##_alloc() {\
	return new(type##_t);\
}

// c++ equiv of void ::operator delete(void *)
#define DEFAULT_FREE(type)\
void type##_free(type##_t * const obj) {\
	delete(obj);\
}

//c++ equiv of "new str(fmt, ...)"
//calls _alloc and then calls _init*
#define DEFAULT_NEW(type)\
type##_t * type##_new() {\
	type##_t * obj = type##_alloc();\
	type##_init(obj);\
	return obj;\
}

// _del calls _destroy and then _free
// c++ equiv of "delete str"
#define DEFAULT_DEL(type)\
void type##_del(type##_t * const o) {\
	if (o) type##_destroy(o);\
	type##_free(o); /*_del behavior:*/\
}


//move.h


#define MAKE_MOVE(returnType, objType, funcName)\
returnType##_t * objType##_##funcName##_move(objType##_t * const obj) {\
	returnType##_t * const result = objType##_##funcName(obj);\
	/*_move behavior: delete incoming objects*/\
	objType##_del(obj);\
	return result;\
}

#define MAKE_MOVE2(returnType, objType, funcName, obj2Type)\
returnType##_t * objType##_##funcName##_move(objType##_t * const obj, obj2Type##_t * const obj2) {\
	returnType##_t * const result = objType##_##funcName(obj, obj2);\
	/*_move behavior: delete incoming objects*/\
	objType##_del(obj);\
	obj2Type##_del(obj2);\
	return result;\
}

#define MAKE_MOVE_VOID(objType, funcName)\
void objType##_##funcName##_move(objType##_t * const obj) {\
	objType##_##funcName(obj);\
	objType##_del(obj);\
}


//str.h


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
STRUCT(str,
	(str, size_t, len, 0),		// len is the blob length (not including the \0 at the end)
	(str, char *, ptr, 1)		// ptr is len+1 in size for strlen strs
)


//_init is for in-place init / is the ctor
void str_init_c(str_t * s, char const * cstr);
void str_init(str_t * s, char const * fmt, ...);

//_destroy is in-place / object dtors
void str_destroy(str_t * s);

//_new calls _init for heap allocated objects


// C functions
str_t * str_new_c(char const * cstr);
str_t * str_new(char const * fmt, ...);

//_move means free args after you're done
str_t * str_cat_move(str_t * a, str_t * b);


//tostr.h


#define DEFAULT_TOSTR(type)\
str_t * type##_tostr(\
	type##_t const * const obj\
) {\
	str_t * s = str_new_c(#type);\
	if (!obj) {\
		return str_cat_move(s, str_new_c("NULL"));\
	}\
	s = str_cat_move(s, str_new("%p={", obj));\
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


//fail.cpp


void fail_cstr(char const * const s) {
	fprintf(stderr, "%s\n", s);
	exit(1);
}

#define fail(...)	(fail_cstr(str_new(__VA_ARGS__)->ptr))	//leaks the allocated string


//safealloc.cpp


void * safealloc(size_t size) {
	void * const ptr = calloc(size, 1);
	if (!ptr) {
		fail("malloc failed for %u bytes\n", size);
		return NULL;
	}
	return ptr;
}


//str.cpp


//can't forward va-args, so ... 
//  says: https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
#define str_init_body() {\
	assert(s);\
	/*should we assert the mem in is dirty, or should we assert it is initialized and cleared?*/\
	assert(!s->ptr);\
\
	va_list args, copy;\
	va_start(args, fmt);\
	va_copy(copy, args);\
	int len = vsnprintf(NULL, 0, fmt, args);\
	if (len < 0) fail_cstr("vsnprintf failed");\
	va_end(copy);\
\
	char * const ptr = newarray(char, len + 1);\
	vsnprintf(ptr, len + 1, fmt, copy);\
	va_end(args);\
\
	s->len = len;\
	s->ptr = ptr;\
}

//allocate members
//c++ eqiv of constructor: str::str
void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//allocate members
//c++ eqiv of constructor: str::str
//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init(str_t * const s, char const * const fmt, ...) {
	str_init_body();
}

//deallocate members
//c++ eqiv of destructor : str::~str
void str_destroy(str_t * const s) {
	if (!s) return;
	delete(s->ptr);
	s->ptr = NULL;
	s->len = 0;
}

DEFAULT_ALLOC(str)		//str_alloc
DEFAULT_FREE(str)		//str_free
DEFAULT_DEL(str)	//str_del

//c++ equiv of "new str(cstr)"
//calls _alloc and then calls _init*
str_t * str_new_c(char const * const cstr) {
	str_t * s = str_alloc();
	str_init_c(s, cstr);	//or in-place init?
	return s;
}

//can't use DEFAULT_NEW since it uses va_list which can't be forwarded
str_t * str_new(char const * const fmt, ...) {
	str_t * s = str_alloc();
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
	str_init_body();
	return s;
}

str_t * str_cat(str_t const * const a, str_t const * const b) {
	str_t * const s = str_alloc();
	s->len = a->len + b->len;	//because for now len includes the null term
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, a->ptr, a->len);
	memcpy(s->ptr + a->len, b->ptr, b->len);
	s->ptr[a->len + b->len] = '\0';
	return s;
}

MAKE_MOVE2(str, str, cat, str)	//str_cat_move from str_cat

void str_println(str_t * const s) {
	printf("%s\n", s->ptr);
}

MAKE_MOVE_VOID(str, println);	//str_println_move from str_println


#if 0
//thread.cpp


typedef struct thread_s {
	pthread_t pthread;
} thread_t;

#define thread_alloc()	new(thread_t)
#define thread_free		delete

void thread_init(
	thread_t * const t,
	void * (*callback)(void *),
	void * arg
) {
}

thread_t * thread_new(
	thread_t * const t,
	void * (*callback)(void *),
	void * arg
) {
	thread_t * t = thread_alloc();
	thread_init(t, callback, arg);
	return t;
}
#endif


//main.cpp


STRUCT(threadInit,
	(threadInit, int, something, 0)
)


//can you turn ((a, b), (c,d)) into (a,b,c,d) in preprocessor?
#if 0	//works, needs DEFER
#define EXPAND2(pack1, pack2)	DEFER pack1, DEFER pack2
void test(
	EXPAND2((int a, int b, int c), (int d, int e))
) {
	printf("%d %d %d %d\n", a, b, c, d);
}
#elif 1

//used to unpack the ()'s
#define DEFER1(x)			DEFER x

// converts macro tuple args into a single expanded list:
// (a, b), (c, d, e), (f) => a, b, c, d, e, f
#define EXPAND(...)\
FOR_EACH(DEFER1, COMMA, __VA_ARGS__)

void test( 
	EXPAND((int a, int b, int c), (int d, int e))
) {
	printf("%d %d %d %d\n", a, b, c, d);
}
#endif

#if 0	//works, but very redundant

DEFAULT_INIT(threadInit)				//threadInit::threadInit
DEFAULT_DESTROY(threadInit)				//threadInit::~threadInit()
DEFAULT_ALLOC(threadInit)				//threadInit::operator new()
DEFAULT_FREE(threadInit)				//threadInit::operator delete()
DEFAULT_NEW(threadInit)					//new threadInit()
DEFAULT_DEL(threadInit)					//delete threadInit()
DEFAULT_TOSTR(threadInit)				//tostring(threadInit)

#elif 1	//works, but looks ugly

#define MAKE_DEFAULT_I(type, method)	CONCAT(DEFAULT_, method)(type)
#define MAKE_DEFAULT(x)		MAKE_DEFAULT_I x
#define MAKE_DEFAULTS(...)\
FOR_EACH(MAKE_DEFAULT, EMPTY, __VA_ARGS__)

MAKE_DEFAULTS(
	(threadInit, INIT),
	(threadInit, DESTROY),
	(threadInit, ALLOC),
	(threadInit, FREE),
	(threadInit, NEW),
	(threadInit, DEL),
	(threadInit, TOSTR))

#else	//needs macro-for extra args

//#define MAKE_DEFAULT	

#define MAKE_DEFAULTS(type, ...)\
FOR_EACH(MAKE_DEFAULT(type), EMPTY, __VA_ARGS__)

MAKE_DEFAULTS(threadInit, INIT, DESTROY, ALLOC, FREE, NEW, DEL, TOSTR)

#endif

MAKE_MOVE(str, threadInit, tostr)		// make threadInit_tostr_move from threadInit_tostr


STRUCT(threadEnd,
	(threadEnd, int, somethingElse, 0))

DEFAULT_INIT(threadEnd)					//threadEnd::threadEnd
DEFAULT_DESTROY(threadEnd)				//threadEnd::~threadEnd
DEFAULT_ALLOC(threadEnd)				//threadEnd::operator new()
DEFAULT_FREE(threadEnd)					//threadEnd::operator delete
DEFAULT_NEW(threadEnd)					//new threadEnd
DEFAULT_DEL(threadEnd)					//delete threadEnd
DEFAULT_TOSTR(threadEnd)				//tostring(threadEnd)
MAKE_MOVE(str, threadEnd, tostr)		// make threadEnd_tostr_move from threadEnd_tostr


void * threadStart(void * arg_) {
	assert(arg_);
	str_println_move(str_cat_move(str_new_c("starting thread with "), threadInit_tostr_move((threadInit_t *)arg_)));
	arg_ = NULL;

	threadEnd_t * const ret = threadEnd_new();
	ret->somethingElse = 53;
	{
		str_t * s = threadEnd_tostr(ret);
		printf("ending thread and returning %s\n", s->ptr);
		str_del(s);
	}
	return ret;
}

int main() {
	int err = 0;

	pthread_t th;
	{
		//pass this to pthread_create, expect it to free this once it's done
		threadInit_t * const initArg = threadInit_new();
		initArg->something = 42;
		
		str_println_move(str_cat_move(str_new_c("creating threadArg_t "), threadInit_tostr(initArg)));
		
		err = pthread_create(&th, NULL, threadStart, (void*)initArg);
		if (err) fail("pthread_create failed with error %d\n", err);
	}
	
	void * ret = NULL;
	err = pthread_join(th, &ret);
	if (err) fail("pthread_join failed with error %d\n", err);

	str_println_move(
		str_cat_move(
			str_new_c("pthread_join succeeded with ret="),
			threadEnd_tostr_move((threadEnd_t*)ret)
		)
	);
	ret = NULL;

	printf("sizeof(str_fieldType_0)=%lu\n", sizeof(str_fieldType_0));

	return 0;
}
