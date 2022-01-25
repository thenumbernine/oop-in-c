#include <assert.h>	//assert
#include <stddef.h>	//size_t

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

//arg of our thread_t init
VTABLE(threadInit,
	(alloc, threadInit_t *, ()),
	(free, void, (threadInit_t *)),
	(destroy, void, (threadInit_t *)),
	(init, void, (threadInit_t *)),
	(tostr, str_t *, (threadInit_t const *))
)
STRUCT(threadInit,
	(threadInit_vtable_t *, v, 0),
	(int, something, 1)
)
MAKE_DEFAULTS(threadInit, ALLOC, FREE, DESTROY, INIT, TOSTR, NEW, DELETE)
MAKE_MOVE(str_t *, threadInit, tostr)


//return value for our thread_t routine
VTABLE(threadEnd,
	(alloc, threadEnd_t *, ()),
	(free, void, (threadEnd_t *)),
	(destroy, void, (threadEnd_t *)),
	(init, void, (threadEnd_t *)),
	(tostr, str_t *, (threadEnd_t const *))
)
STRUCT(threadEnd,
	(int, somethingElse, 0)
)
MAKE_DEFAULTS(threadEnd, ALLOC, FREE, DESTROY, INIT, TOSTR, NEW, DELETE)
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

	//TODO how to call a member method without referencing the object twice ...

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
