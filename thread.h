#pragma once

typedef void *(*threadStart_t)(void *);

STRUCT(thread,
	(thread, pthread_t, pthread, 0),
	(thread, void*, arg, 1))

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


//(void*, arg) => void * arg
#define UNPACK2(a, b)	a b
#define MAKE_NEW_FOR_INIT_ARGS(tuple, extra)	UNPACK2 tuple

#define TUPLE_ARG2(a, b)	b
#define MAKE_NEW_FOR_INIT_CALL(tuple, extra)	TUPLE_ARG2 tuple

#define MAKE_NEW_FOR_INIT(type, initSuffix, ...)\
type##_t * type##_new##initSuffix(\
FOR_EACH(MAKE_NEW_FOR_INIT_ARGS, COMMA, EMPTY, __VA_ARGS__)\
) {\
	type##_t * obj = type##_alloc();\
	type##_init(obj,\
FOR_EACH(MAKE_NEW_FOR_INIT_CALL, COMMA, EMPTY, __VA_ARGS__)\
	);\
	return obj;\
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
