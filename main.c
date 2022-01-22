#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>	//offsetof
#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <pthread.h>


///macros.h


#define numberof(x)		(sizeof(x)/sizeof(*(x)))
#define endof(x)		((x) + numberof(x))

//new.h


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)

void delete(void * ptr) {
	if (ptr) free(ptr);
}


//reflect.h


#define STRUCT_BEGIN(type)						typedef struct type##_s {
#define STRUCT_FIELD(structName, type, name)		type name;
#define STRUCT_END(type)						} type##_t;


typedef struct reflect_s {
	size_t offset;
	size_t size;
	char * name;
} reflect_t;

#define STRUCT_REFL_BEGIN(type)								reflect_t type##_fields[] = {
#define STRUCT_REFL_FIELD(structName, fieldType, fieldName)		{ .offset=offsetof(structName##_t, fieldName), .size=sizeof(fieldType), .name=#fieldName},
#define STRUCT_REFL_END(type)								};

//TODO if we can replace "fields" here with STRUCT_FIELD(fields) (though this requires picking out the 1st, 2nd, and 3rd
// then we can just call all "fields" again on STRUCT_FIELD_REFL 
// and we don't have to have two declarations per struct
#define STRUCT(type, fields)\
STRUCT_BEGIN(type) \
fields /* a space before the wrap-line here is necessary*/ \
STRUCT_END(type)

#define STRUCT_REFL(type, fields)\
STRUCT_REFL_BEGIN(type) \
fields \
STRUCT_REFL_END(type)

//move.h


#define MAKE_MOVE(returnType, objType, funcName)\
returnType##_t * objType##_##funcName##_move(objType##_t * const obj) {\
	returnType##_t * const result = objType##_##funcName(obj);\
	objType##_del(obj);\
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
	STRUCT_FIELD(str, size_t, len)		//len is the blob length (not including the \0 at the end)
	STRUCT_FIELD(str, char *, ptr))		//ptr is len+1 in size for strlen strs
STRUCT_REFL(str,
	STRUCT_REFL_FIELD(str, size_t, len)
	STRUCT_REFL_FIELD(str, char *, ptr))

//_init is for in-place init / is the ctor
void str_init_c(str_t * const s, char const * const cstr);
void str_init(str_t * const s, char const * const fmt, ...);

//_dtor is in-place / object dtors
void str_dtor(str_t * const s);

//_new calls _init for heap allocated objects


// C functions
str_t * str_new_c(char const * const cstr);
str_t * str_new(char const * const fmt, ...);

//_move means free args after you're done
str_t * str_cat_move(str_t * const a, str_t * const b);


//tostr.h


str_t * default_tostr(
	void * const obj,
	char const * const structName,
	reflect_t * const fields,
	size_t const numFields
) {
	str_t * s = str_new_c(structName);
	if (!obj) {
		return str_cat_move(s, str_new_c("NULL"));
	}
	s = str_cat_move(s, str_new("%p={", obj));
	reflect_t * endOfFields = fields + numFields;
	for (reflect_t * field = fields; field < endOfFields; ++field) {
	}
	s = str_cat_move(s, str_new_c("}"));
	return s;
}

#define MAKE_TOSTR(type)\
str_t * type##_tostr(type##_t const * const obj) {\
	return default_tostr((void*)obj, #type, type##_fields, numberof(type##_fields));\
}



//fail.cpp


void fail_cstr(char const * const s) {
	fprintf(stderr, "%s\n", s);
	exit(1);
}

#define fail(args...)	(fail_cstr(str_new(args)->ptr))	//leaks the allocated string


//safealloc.cpp


void * safealloc(size_t size) {
	void * ptr = calloc(size, 1);
	if (!ptr) {
		fail("malloc failed for %u bytes\n", size);
		return NULL;
	}
	return ptr;
}


//str.cpp

//can't forward va-args, so ... 
//  says: https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
#define str_init_body {\
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
	str_init_body
}

//deallocate members
//c++ eqiv of destructor : str::~str
void str_dtor(str_t * const s) {
	if (!s) return;
	delete(s->ptr);
	s->ptr = NULL;
	s->len = 0;
}


//class allocator -- for returning the  memory of the class
// c++ equiv of void * ::operator new(size_t)
#define str_alloc()		new(str_t)

// c++ equiv of void ::operator delete(void *)
#define str_free		delete

//c++ equiv of "new str(cstr)"
//calls _alloc and then calls _init*
str_t * str_new_c(char const * const cstr) {
	str_t * s = str_alloc();
	str_init_c(s, cstr);	//or in-place init?
	return s;
}

//c++ equiv of "new str(fmt, ...)"
//calls _alloc and then calls _init*
str_t * str_new(char const * const fmt, ...) {
	str_t * s = str_alloc();
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
	str_init_body
	return s;
}

// _del calls _dtor and then _free
// c++ equiv of "delete str"
#define MAKE_DEL(type)\
void type##_del(type##_t * const o) {\
	if (o) type##_dtor(o);\
\
	/*_del behavior:*/\
	type##_free(o);\
}

MAKE_DEL(str)	//str_del calls str_dtor and then free()

str_t * str_cat_move(str_t * const a, str_t * const b) {
	str_t * const s = str_alloc();
	s->len = a->len + b->len;	//because for now len includes the null term
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, a->ptr, a->len);
	memcpy(s->ptr + a->len, b->ptr, b->len);
	s->ptr[a->len + b->len] = '\0';

	//_move behavior: delete incoming objects
	str_del(a);
	str_del(b);

	return s;
}

void str_println(str_t * const s) {
	printf("%s\n", s->ptr);
}

MAKE_MOVE_VOID(str, println);


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
	STRUCT_FIELD(threadInit, int, something))
STRUCT_REFL(threadInit,
	STRUCT_REFL_FIELD(threadInit, int, something))

#define threadInit_dtor(t)
#define threadInit_free	delete
MAKE_DEL(threadInit)
MAKE_TOSTR(threadInit)				//
MAKE_MOVE(str, threadInit, tostr)	// make threadInit_tostr_move from threadInit_tostr


STRUCT(threadEnd,
	STRUCT_FIELD(threadEnd, int, somethingElse))
STRUCT_REFL(threadEnd,
	STRUCT_REFL_FIELD(threadEnd, int, somethingElse))

#define threadEnd_dtor(t)			//threadEnd_dtor == threadEnd::~threadEnd
#define threadEnd_free	delete		//threadEnd_free == threadEnd::operator delete
MAKE_DEL(threadEnd)					//threadEnd_del == delete threadEnd
MAKE_TOSTR(threadEnd)				//threadEnd_tostr == tostring(threadEnd)
MAKE_MOVE(str, threadEnd, tostr)	// make threadEnd_tostr_move from threadEnd_tostr


void * threadStart(void * arg_) {
	assert(arg_);
	str_println_move(str_cat_move(str_new_c("starting thread with "), threadInit_tostr_move((threadInit_t *)arg_)));
	arg_ = NULL;

	threadEnd_t * const ret = new(threadEnd_t);
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
		threadInit_t * const initArg = new(threadInit_t);
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

	return 0;
}
