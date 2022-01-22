#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <pthread.h>


//new.h


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)


void default_del(void * ptr) {
	if (!ptr) return;
	free(ptr);
}

//str.h


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
typedef struct str_s {
	size_t len;		//len is the blob length (not including the \0 at the end)
	char * ptr;		//ptr is len+1 in size for strlen strs
} str_t;

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

void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init(str_t * const s, char const * const fmt, ...) {
	str_init_body
}

void str_dtor(str_t * const s) {
	if (!s) return;
	if (s->ptr) free(s->ptr);
	s->ptr = NULL;
	s->len = 0;
}


str_t * str_new_c(char const * const cstr) {
	str_t * s = new(str_t);
	str_init_c(s, cstr);	//or in-place init?
	return s;
}

str_t * str_new(char const * const fmt, ...) {
	str_t * s = new(str_t);
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
	str_init_body
	return s;
}

/* _del calls _dtor and then frees memory */
#define MAKE_DEL(type)\
void type##_del(type##_t * const o) {\
	if (!o) return;\
	type##_dtor(o);\
\
	/*_del behavior:*/\
	free(o);\
}

MAKE_DEL(str)	//str_del calls str_dtor and then free()

str_t * str_cat_move(str_t * const a, str_t * const b) {
	str_t * const s = new(str_t);
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

void str_println_move(str_t * const s) {
	printf("%s\n", s->ptr);
	
	//_move
	str_del(s);
}


//main.cpp


typedef struct threadInit_s {
	int something;
} threadInit_t;

#define threadInit_del	default_del

str_t * threadInit_tostr(threadInit_t const * const t) {
	if (!t) return str_new_c("(threadInit_t*)NULL");
	return str_new("(threadInit_t*)%p={something=%d}", t, t->something);
}

str_t * threadInit_tostr_move(threadInit_t * const t) {
	str_t * s = threadInit_tostr(t);
	threadInit_del(t);
	return s;
}


typedef struct threadEnd_s {
	int somethingElse;
} threadEnd_t;

#define threadEnd_del	default_del

str_t * threadEnd_tostr(threadEnd_t const * const t) {
	if (!t) return str_new_c("threadEnd_t*)NULL");
	return str_new("(threadEnd_t*)%p={somethingElse=%d}", t, t->somethingElse);
}

str_t * threadEnd_tostr_move(threadEnd_t * const t) {
	str_t * s = threadEnd_tostr(t);
	threadEnd_del(t);
	return s;
}


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
