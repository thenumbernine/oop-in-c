#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <pthread.h>


//new.h


#define new(x)			(x*)safealloc(sizeof(x))
#define newarray(x,len)	(x*)safealloc(sizeof(x) * len)


//str.h


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
typedef struct str_s {
	size_t len;		//len is the blob length (not including the \0 at the end)
	char * ptr;		//ptr is len+1 in size for strlen strs
} str_t;

//_init is for in-place init
void str_init_c(str_t * const s, char const * const cstr);
void str_init(str_t * const s, char const * const fmt, ...);

//_dtor is in-place / object dtors
void str_dtor(str_t * const s);

//_new calls _init for heap allocated objects
str_t * str_new_c(char const * const cstr);
str_t * str_new(char const * const fmt, ...);

//https://stackoverflow.com/questions/10405436/anonymous-functions-using-gcc-statement-expressions
#define lambda(return_type, function_body) \
({\
	  return_type __fn__ function_body \
	  __fn__; \
})

#if 0
#define str_new(args...)\
({\
	str_t * s = new(str_t);\
	str_init(s, args);\
	s;\
})

#define str_new_c(args...)\
({\
	str_t * s = new(str_t);\
	str_init_c(s, args);\
	s;\
})
#endif

//_del calls _dtor and then frees memory
void str_del(str_t * s);

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
	void * ptr = malloc(size);
	if (!ptr) {
		fail("malloc failed for %u bytes\n", size);
		return NULL;
	}
	return ptr;
}


//str.cpp


void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init(str_t * const s, char const * const fmt, ...) {
	assert(s);
	assert(!s->ptr);	//should we assert the mem in is dirty, or should we assert it is initialized and cleared?

	va_list args, copy;
	va_start(args, fmt);
	va_copy(copy, args);
	int len = vsnprintf(NULL, 0, fmt, args);
	if (len < 0) fail_cstr("vsnprintf failed");
	va_end(copy);

	char * const ptr = newarray(char, len + 1);
	vsnprintf(ptr, len + 1, fmt, copy);
	va_end(args);

	s->len = len;
	s->ptr = ptr;
}

void str_dtor(str_t * const s) {
	if (!s) return;
	if (s->ptr) free(s->ptr);
	s->ptr = NULL;
	s->len = 0;
}


#if 1
str_t * str_new_c(char const * const cstr) {
	str_t * s = new(str_t);
	str_init_c(s, cstr);	//or in-place init?
	return s;
}

//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
str_t * str_new(char const * const fmt, ...) {
	//can't forward va-args so copy the above body ...
	
	va_list args, copy;
	va_start(args, fmt);
	va_copy(copy, args);
	int len = vsnprintf(NULL, 0, fmt, args);
	if (len < 0) fail_cstr("vsnprintf failed");
	va_end(copy);

	char * const ptr = newarray(char, len + 1);
	vsnprintf(ptr, len + 1, fmt, copy);
	va_end(args);

	str_t * s = new(str_t);
	s->len = len;
	s->ptr = ptr;
	return s;
}
#endif

void str_del(str_t * const s) {
	if (!s) return;
	str_dtor(s);
	
	//_del behavior:
	free(s);
}

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

str_t * threadInit_tostr(threadInit_t const * const t) {
	if (!t) return str_new_c("(threadInit_t*)NULL");
	return str_new("(threadInit_t*)%p={something=%d}", t, t->something);
}

typedef struct threadEnd_s {
	int somethingElse;
} threadEnd_t;

str_t * threadEnd_tostr(threadEnd_t const * const t) {
	if (!t) return str_new_c("threadEnd_t*)NULL");
	return str_new("(threadEnd_t*)%p={somethingElse=%d}", t, t->somethingElse);
}

void * threadStart(void * arg_) {
	assert(arg_);
	{
		threadInit_t * const arg = (threadInit_t *)arg_;
		str_println_move(str_cat_move(str_new_c("starting thread with "), threadInit_tostr(arg)));
		free(arg);
		arg_ = NULL;
	}

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

	{
		threadEnd_t * const argret = (threadEnd_t*)ret;
		ret = NULL;
		str_println_move(
			str_cat_move(
				str_new_c("pthread_join succeeded with ret="),
				threadEnd_tostr(argret)
			)
		);
		free(argret);
	}

	return 0;
}
