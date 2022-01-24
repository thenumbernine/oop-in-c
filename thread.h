#pragma once

typedef struct thread_s {
	pthread_t pthread;
	void * arg;
} thread_t;

DEFAULT_ALLOC(thread)
DEFAULT_FREE(thread)
DEFAULT_DESTROY(thread)
DEFAULT_DELETE(thread)

void thread_init(
	thread_t * const t,
	// extra args forwarded
	void * (*threadStart)(void *),
	void * arg
) {
	t->arg = arg;
	int err = pthread_create(&t->pthread, NULL, threadStart, (void*)t);
	if (err) fail("pthread_create failed with error %d\n", err);
}

thread_t * thread_new(
	// extra args forwarded
	void * (*threadStart)(void *),
	void * arg
) {
	thread_t * t = thread_alloc();
	thread_init(t, threadStart, arg);
	return t;
}

void * thread_join(
	thread_t * const t
) {
	void * ret = NULL;
	int err = pthread_join(t->pthread, &ret);
	if (err) fail("pthread_join failed with error %d\n", err);
	return ret;
}
