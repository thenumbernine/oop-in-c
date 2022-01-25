#pragma once

typedef void *(*threadStart_t)(void *);

#if 0
STRUCT(thread,
	(thread, pthread_t, pthread, 0),
	(thread, void*, arg, 1))
#else
STRUCT2(thread,
	(pthread_t, pthread, 0),
	(void*, arg, 1))
#endif

MAKE_DEFAULTS(thread, ALLOC, FREE, DESTROY, DELETE)

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
