#pragma once

typedef void *(*threadStart_t)(void *);

typedef struct thread_s thread_t;

typedef struct {
	thread_t * (*alloc)();
	void (*free)(thread_t *);
	void (*destroy)(thread_t *);
	void (*init)(thread_t *, threadStart_t threadStart, void * arg);
	void * (*join)(thread_t *);
	void * (*join_move)(thread_t *);
} thread_vtable_t;
extern thread_vtable_t thread_vtable;

STRUCT(thread,
	(thread_vtable_t *, v, 0),
	(pthread_t, pthread, 1),
	(void*, arg, 2))

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

MAKE_MOVE(void*, thread, join);	//thread_join_move ... joins and deletes thread

thread_vtable_t thread_vtable = {
	.alloc = thread_alloc,
	.free = thread_free,
	.destroy = thread_destroy,
	.init = thread_init,
	.join = thread_join,
	.join_move = thread_join_move,
};
