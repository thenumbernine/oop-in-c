#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <pthread.h>


void * safealloc(size_t size);

#include "struct.h"
#include "class.h"//needs str but comes before str
#include "move.h"
#include "fail.h"
#include "str.h"
#include "thread.h"
#include "file.h"

void * safealloc(size_t size) {
	void * const ptr = calloc(size, 1);
	if (!ptr) {
		fail("malloc failed for %u bytes\n", size);
		return NULL;
	}
	return ptr;
}



#if 0
//can you turn ((a, b), (c,d)) into (a,b,c,d) in preprocessor?
void test( 
	EXPAND((int a, int b, int c), (int d, int e))
) {
	printf("%d %d %d %d %d\n", a, b, c, d, e);
}
//ok now can you use this in a FOR_EACH
#endif
#if 1 //yeah but can you defer?
#define TEST2(a,b,c,d) 		void test(a, b, c, d)
#define CONCATV1(e,f,g,h) 	TEST2(e,f,g,h)
#define CONCATV(tuple) 		CONCATV1(tuple)	//looks like to use the expanded args you just gotta defer once like so
#define TEST(tuple, extra) 	CONCATV(EXPAND(tuple, (extra)))
TEST((int a, int b, int c), int d) {
	printf("%d %d %d %d\n", a, b, c, d);
}
#endif
#if 1 //ok so defer works, how about defer expand for loop?
#define MAKEFUNC(suffix, extra)	void func_##suffix(int n, int extra) {}
FOR_EACH(MAKEFUNC, EMPTY, everyoneGetsIt, a, b, c)
#endif


//arg of our thread_t init
STRUCT(threadInit,
	(int, something, 0)
)
MAKE_DEFAULTS(threadInit, INIT, DESTROY, ALLOC, FREE, NEW, DELETE, TOSTR)
MAKE_MOVE(str_t *, threadInit, tostr)


//return value for our thread_t routine
STRUCT(threadEnd,
	(int, somethingElse, 0)
)
MAKE_DEFAULTS(threadEnd, INIT, DESTROY, ALLOC, FREE, NEW, DELETE, TOSTR)
MAKE_MOVE(str_t *, threadEnd, tostr)


void * threadStart(void * arg_) {
	assert(arg_);
	thread_t * const this = (thread_t *)arg_;
	arg_ = NULL;

#if 1
	str_println_move(
		str_cat_move(
			str_new_c("starting thread with "),
			threadInit_tostr_move((threadInit_t *)this->arg)
		)
	);
#else
	
	call(
		str_new_c("starting thread with "),
		"cat_move",
		call(
			(threadInit_t*)this->arg,
			"tostr_move"
		)
	)

#endif

	threadEnd_t * const ret = threadEnd_new();
	ret->somethingElse = 53;
	{
		str_t * s = threadEnd_tostr(ret);
		printf("ending thread and returning %s\n", s->ptr);
		str_delete(s);
	}
	return ret;
}

int main() {
	//int err = 0;

	void * ret = NULL;
	//pthread_t th;
	{
		//pass this to pthread_create, expect it to free this once it's done
		threadInit_t * const initArg = threadInit_new();
		initArg->something = 42;
		
		str_println_move(str_cat_move(str_new_c("creating threadArg_t "), threadInit_tostr(initArg)));
		
		thread_t * t = thread_new(threadStart, (void*)initArg);
		//err = pthread_create(&th, NULL, threadStart, (void*)initArg);
		//if (err) fail("pthread_create failed with error %d\n", err);
	
		//err = pthread_join(th, &ret);
		//if (err) fail("pthread_join failed with error %d\n", err);
		ret = thread_join_move(t);
	}

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
