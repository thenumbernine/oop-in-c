#pragma once

#include <pthread.h>
#include <sys/sysinfo.h>	//get_nprocs_conf

typedef void *(*threadStart_t)(void *);

MAKE_TOSTRING_FOR_ADDR(pthread_t)
MAKE_TYPEINFO(pthread_t)

#define CLASS_thread_fields (\
	(pthread_t, pthread),\
	(voidp_t, arg)\
)
#define CLASS_thread_methods (\
	(alloc, thread_t *, ()),\
	(free, void, (thread_t *)),\
	(destroy, void, (thread_t *)),\
	(init, void, (thread_t *, threadStart_t threadStart, void * arg)),\
	(join, void *, (thread_t *)),\
	(join_move, void *, (thread_t *))\
)
CLASS(thread)
MAKE_DEFAULTS(thread, ALLOC, FREE, DESTROY, TOSTRING)

#define ASSERTZERO(func, ...) \
{\
	int err = func(__VA_ARGS__);\
	if (err) fail(#func " failed with error %d", err);\
}

void thread_init(
	thread_t * const t,
	// extra args forwarded
	threadStart_t threadStart,
	void * arg/*,
	size_t stackSize
	*/
) {
	t->arg = arg;
	
	pthread_attr_t attr;
	ASSERTZERO(pthread_attr_init, &attr);

	size_t stackSize = 0;
	if (stackSize) {
		ASSERTZERO(pthread_attr_setstacksize, &attr, stackSize);
	}

	int numCores = get_nprocs_conf();
	//printf("number of cores: %d\n", numCores);	// 16
	//printf("sizeof(cpu_set_t) = %lu\n", sizeof(cpu_set_t));	// 128

	//https://linux.die.net/man/3/cpu_set
	{
		cpu_set_t * cpusetp = CPU_ALLOC(numCores);
		if (!cpusetp) fail("CPU_ALLOC(%d) failed", numCores);

		size_t cpuAllocSize = CPU_ALLOC_SIZE(numCores);
		CPU_ZERO_S(cpuAllocSize, cpusetp);
		for (int i = 0; i < numCores; ++i) {
			CPU_SET_S(i, cpuAllocSize, cpusetp);
		}
		
		ASSERTZERO(pthread_attr_setaffinity_np, &attr, cpuAllocSize, cpusetp);
		CPU_FREE(cpusetp);
	}

	ASSERTZERO(pthread_create, &t->pthread, &attr, threadStart, (void*)t);
   	ASSERTZERO(pthread_attr_destroy, &attr);
}

void * thread_join(
	thread_t * const t
) {
	void * ret = NULL;
	ASSERTZERO(pthread_join, t->pthread, &ret);
	return ret;
}

MAKE_MOVE(void *, thread, join);	//thread_join_move ... joins and deletes thread
