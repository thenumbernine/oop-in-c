#pragma once

#include <pthread.h>
#include <sys/sysinfo.h>	//get_nprocs_conf

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

#define ASSERTZERO(func, ...) \
{\
	int err = func(__VA_ARGS__);\
	if (err) fail(#func " failed with error %d", err);\
}

void thread_init(
	thread_t * const t,
	// extra args forwarded
	threadStart_t threadStart,
	void * arg
) {
	t->arg = arg;
	
	pthread_attr_t attrib;
	ASSERTZERO(pthread_attr_init, &attrib);

	size_t stackSize = 0;
	if (stackSize) {
		ASSERTZERO(pthread_attr_setstacksize, &attrib, stackSize);
	}

	int numCores = get_nprocs_conf();
	printf("number of cores: %d\n", numCores);									// 16
	printf("sizeof(cpu_set_t) = %lu\n", sizeof(cpu_set_t));						// 128
#if 0	
	for (int i = 0; i <= numCores; ++i) {
		printf("CPU_ALLOC_SIZE(%d) = %lu\n", i, CPU_ALLOC_SIZE(i));				// 0 => 0, all else => 8 ... is it a pointer?
	}
#endif

#if 1
#if 0	//limiting number of cores to zero stops the leak
	numCores = numCores > 0 ? 0 : numCores;
#endif	
	
	cpu_set_t * cpusetp = CPU_ALLOC(numCores);
	if (!cpusetp) fail("CPU_ALLOC(%d) failed", numCores);

	size_t cpuAllocSize = CPU_ALLOC_SIZE(numCores);
	CPU_ZERO_S(cpuAllocSize, cpusetp);

	for (int i = 0; i < numCores; ++i) {
		CPU_SET_S(i, cpuAllocSize, cpusetp);
	}

#if 1	//valgrind says this leaks
	ASSERTZERO(pthread_attr_setaffinity_np, &attrib, cpuAllocSize, cpusetp);
#endif
#if 0	//call but set zero to size to stop the leak 
	ASSERTZERO(pthread_attr_setaffinity_np, &attrib, 0, cpuset);
#endif
#endif
#if 0	//same as above but without heap alloc
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	for (int i = 0; i < numCores; ++i) {
		CPU_SET(i, &cpuset);
	}
	ASSERTZERO(pthread_attr_setaffinity_np, &attrib, sizeof(cpu_set_t), &cpuset);
#endif

	ASSERTZERO(pthread_create, &t->pthread, &attrib, threadStart, (void*)t);
   	ASSERTZERO(pthread_attr_destroy, &attrib);
	
#if 0	//valgrind says this leaks
	ASSERTZERO(pthread_setaffinity_np, t->pthread, cpuAllocSize, cpusetp);
#endif
#if 1
	CPU_FREE(cpusetp);
#endif
}
MAKE_NEW_FOR_INIT(thread, ,
	(threadStart_t, threadStart),
	(void *, arg))

void * thread_join(
	thread_t * const t
) {
	void * ret = NULL;
	ASSERTZERO(pthread_join, t->pthread, &ret);
	return ret;
}

MAKE_MOVE(void*, thread, join);	//thread_join_move ... joins and deletes thread
