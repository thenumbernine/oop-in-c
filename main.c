//these have to go first or gcc can't find these macros
#define _GNU_SOURCE
#include <sched.h>		//CPU_ALLOC CPU_FREE CPU_SET

#include <assert.h>	//assert
#include <stddef.h>	//size_t

void * safealloc(size_t size);

#include "struct.h"
#include "class.h"//needs string but comes before string
#include "move.h"
#include "fail.h"
#include "object.h"
#include "string.h"
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

//arg of our thread_t init

#define CLASS_threadInit_fields (\
	(int, something, 1)\
)
#define CLASS_threadInit_methods (\
	(alloc, threadInit_t *, ()),\
	(free, void, (threadInit_t *)),\
	(destroy, void, (threadInit_t *)),\
	(init, void, (threadInit_t *)),\
	(tostring, string_t *, (threadInit_t const *))\
)
CLASS(threadInit)
MAKE_DEFAULTS(threadInit, ALLOC, FREE, DESTROY, INIT, TOSTRING)
MAKE_MOVE(string_t *, threadInit, tostring)


//return value for our thread_t routine

#define CLASS_threadEnd_fields (\
	(int, somethingElse, 1)\
)
#define CLASS_threadEnd_methods (\
	(alloc, threadEnd_t *, ()),\
	(free, void, (threadEnd_t *)),\
	(destroy, void, (threadEnd_t *)),\
	(init, void, (threadEnd_t *)),\
	(tostring, string_t *, (threadEnd_t const *))\
)
CLASS(threadEnd)
MAKE_DEFAULTS(threadEnd, ALLOC, FREE, DESTROY, INIT, TOSTRING)
MAKE_MOVE(string_t *, threadEnd, tostring)


void * threadStart(void * arg_) {
	assert(arg_);
	thread_t * const this = (thread_t *)arg_;
	arg_ = NULL;

#if 1
	string_println_move(
		string_cat_move(
			newobj(string,_c,"starting thread with "),
			threadInit_tostring_move((threadInit_t *)this->arg)
		)
	);
#else

	//TODO how to call a member method without referencing the object twice ...

	call(
		newobj(string,_c,"starting thread with "),
		"cat_move",
		call(
			(threadInit_t*)this->arg,
			"tostring_move"
		)
	)

#endif

	threadEnd_t * const ret = newobj(threadEnd,);
	ret->somethingElse = 53;
	{
		string_t * s = threadEnd_tostring(ret);
		printf("ending thread and returning %s\n", s->ptr);
		deleteobj(s);
	}
	return ret;
}

int main() {
	//int err = 0;

	void * ret = NULL;
	{
		//pass this to pthread_create, expect it to free this once it's done
		threadInit_t * const initArg = newobj(threadInit,);
		initArg->something = 42;
		string_println_move(string_cat_move(newobj(string,_c,"creating threadArg_t "), threadInit_tostring(initArg)));
		ret = thread_join_move(newobj(thread,, threadStart, (void*)initArg));
	}

	string_println_move(
		string_cat_move(
			newobj(string,_c,"pthread_join succeeded with ret="),
			threadEnd_tostring_move((threadEnd_t*)ret)
		)
	);
	ret = NULL;

	printf("sizeof(string_ptr_fieldType)=%lu\n", sizeof(string_ptr_fieldType));

	return 0;
}
