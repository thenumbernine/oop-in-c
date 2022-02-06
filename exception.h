#pragma once

#include <setjmp.h>
#include "vector.h"	//vector_t
#include "class.h"	//newobj

/*
TRY {
	do something
	THROW(1)
} CATCH(1) {
} ENDTRY
*/



// can't use 'newobj' and gcc lambda in global scope
// can't use newobj_func because vector_init_ptr is not arg-less default _init
#if 0	
vector_t * exceptionJmpBufStack = newobj(vector,_ptr,sizeof(jmp_buf),NULL,0);
#endif
#if 1
vector_t * exceptionJmpBufStack = NULL;
object_t * exceptionThrownObj = NULL;
void staticInit_exceptionJmpBufStack() {
#if 0	// using a generic 'newobj_func' that assumes object_t and object_vtable line up with vector_t and vector_vtable ...
	exceptionJmpBufStack = (vector_t*)newobj_func((object_vtable_t*)&vector_vtable);
	exceptionJmpBufStack->elemSize = sizeof(jmp_buf);
#endif
#if 1	// just inline the vector functions
	exceptionJmpBufStack = vector_vtable.alloc();
	exceptionJmpBufStack->v = &vector_vtable;
	exceptionJmpBufStack->v->init_ptr(exceptionJmpBufStack, sizeof(jmp_buf), NULL, 0);
#endif
}
void staticDestroy_exceptionJmpBufStack() {
	deleteobj(exceptionJmpBufStack);
	exceptionJmpBufStack = NULL;
}
#endif


#define exceptionJmpBufStackBack()	((jmp_buf*)exceptionJmpBufStack->v->back(exceptionJmpBufStack))


#define TRY \
{\
	exceptionJmpBufStack->v->push_back(exceptionJmpBufStack, NULL);\
	int val = setjmp(*exceptionJmpBufStackBack());\
	switch (val) {\
	case 0:


#define ENDTRY\
		break;\
		default:\
			fail("unhandled exception %d", val);\
		break;\
	}\
	exceptionJmpBufStack->v->pop_back(exceptionJmpBufStack, NULL);\
}


/*
TODO
how to change 'option' from an int to an object
and another how to is how to throw obj references instead of ptrs
maybe I should register all exception classes?
or maybe I could have a static object for which exception to catch 
*/
#define THROW(obj)\
	{\
		exceptionThrownObj = obj;\
		longjmp(*exceptionJmpBufStackBack(), 1);\
	}

#define CATCH(exceptionVar)\
	break;\
	case 1:\
/*		object_t * exceptionVar = exceptionThrownObj; */
