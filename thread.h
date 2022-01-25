#pragma once

#include <pthread.h>

typedef void *(*threadStart_t)(void *);

VTABLE(thread,
	(alloc, thread_t *, ()),
	(free, void, (thread_t *)),
	(destroy, void, (thread_t *)),
	(init, void, (thread_t *, threadStart_t threadStart, void * arg)),
	(join, void *, (thread_t *)),
	(join_move, void *, (thread_t *)))

STRUCT(thread,
	(thread_vtable_t *, v, 0),
	(pthread_t, pthread, 1),
	(void*, arg, 2))

MAKE_DEFAULTS(thread, ALLOC, FREE, DESTROY, DELETE, TOSTR)

void thread_init(
	thread_t * const t,
	// extra args forwarded
	threadStart_t threadStart,
	void * arg
) {
	t->arg = arg;
	int err = pthread_create(&t->pthread, NULL, threadStart, (void*)t);
	if (err) fail("pthread_create failed with error %d\n", err);
}
MAKE_NEW_FOR_INIT(thread, ,
	(threadStart_t, threadStart),
	(void *, arg))

void * thread_join(
	thread_t * const t
) {
	void * ret = NULL;
	int err = pthread_join(t->pthread, &ret);
	if (err) fail("pthread_join failed with error %d\n", err);
	return ret;
}

MAKE_MOVE(void*, thread, join);	//thread_join_move ... joins and deletes thread
