#pragma once

//TODO how to do templates ...

// which is cleaner design?
#if 0
// specifying fields and methods as macros?
#define CLASS_vector_fields (\
	(size_t, capacity),\
	(bytep_t, data),\
	(size_t, size),\
	(size_t, elemSize)\
)
#define CLASS_vector_methods (\
	(alloc, vector_t *, ()),\
	(free, void, (vector_t *)),\
	(destroy, void, (vector_t *)),\
	(init, void, (vector_t *)),\
	(init_ptr, void, (vector_t *, size_t elemSize, void * data, size_t size)),\
	(push_back, void, (vector_t *, byte const * const what)),\
	(pop_back, void, (vector_t *, byte * const where)),\
	(resize, void, (vector_t *, size_t newSize)),\
	(size, size_t, (vector_t const *)),/* getter */\
	(capacity, size_t, (vector_t const *)),/* getter */\
	(data, bytep_t, (vector_t const *)),/* getter */\
	(reserve, void, (vector_t *, size_t newCapacity)),\
	(back, bytep_t, (vector_t *))\
)
CLASS(vector)

#else
// or specifying them as arguments?
GENERATE_CLASS(
	/* name */
	vector,
	/* fields */
	(
		(size_t, capacity),
		(bytep_t, data),
		(size_t, size),
		(size_t, elemSize)
	),
	/* methods */
	(
		(alloc, vector_t *, ()),
		(free, void, (vector_t *)),
		(destroy, void, (vector_t *)),
		(init, void, (vector_t *)),
		(init_ptr, void, (vector_t *, size_t elemSize, void * data, size_t size)),
		(push_back, void, (vector_t *, byte const * const what)),
		(pop_back, void, (vector_t *, byte * const where)),
		(resize, void, (vector_t *, size_t newSize)),
		(size, size_t, (vector_t const *)),/* getter */
		(capacity, size_t, (vector_t const *)),/* getter */
		(data, bytep_t, (vector_t const *)),/* getter */
		(reserve, void, (vector_t *, size_t newCapacity)),
		(back, bytep_t, (vector_t *))
	)
)
#endif

void vector_destroy(vector_t * const v) {
	if (!v) return;
	// TODO array dtor ... if it is an object type ...
	// but to do this you have to know the type of the vector elem
	// and this implies template type parameters ...
	deleteprim(v->data);
	v->data = NULL;
}

MAKE_DEFAULTS(vector, ALLOC, FREE)

void vector_init(vector_t * const v) {
	//TODO should I init all fields here or should I trust calloc to have cleared them?
	// I will init all here, because this works best for stack allocations
	// actually on second thought ...
	// ... stack allocation can't just use _init, it must also assign the vtable
	// so maybe _init should assign vtable?
	// or maybe I should create a 'newheapobj' vs 'newstackobj',
	//  where 'newheapobj' does _alloc and vtable assignment
	//  while 'newstackobj' just does vtable assignment?
	v->size = 0;
	v->capacity = 0;
	v->data = NULL;
	v->elemSize = 1;
}

/*
elemSize = elemSize.  0 defaults to 1.
initData = initial data to copy in.  set to null not to copy.
initSize = initial size to resize to.
*/
void vector_init_ptr(vector_t * const v, size_t elemSize, voidp_t initData, size_t initSize) {
	v->size = 0;
	v->capacity = 0;
	v->data = NULL;
	if (elemSize == 0) elemSize = 1;
	v->elemSize = elemSize;
	if (initSize) v->v->resize(v, initSize);
	if (initData) memcpy(v->data, initData, elemSize * initSize);
}

// assumes 'what' is of size v->elemSize
void vector_push_back(vector_t * const v, byte const * const what) {
	v->v->resize(v, v->size + 1);
	if (what) {
		memcpy(v->v->back(v), what, v->elemSize);
	}
}

void vector_pop_back(vector_t * v, byte * const where) {
	if (!v->size) fail("cannot pop an empty vector");
	if (where) memcpy(where, v->v->back(v), v->elemSize);
	v->v->resize(v, v->size - 1);
}

void vector_resize(vector_t * const v, size_t newSize) {
	v->v->reserve(v, newSize);
	v->size = newSize;
	assert(v->size <= v->capacity);
}

//hahahah, a getter.
size_t vector_size(vector_t const * const v) {
	return v->size;
}

size_t vector_capacity(vector_t const * const v) {
	return v->capacity;
}

byte * vector_data(vector_t const * const v) {
	return v->data;
}

//TODO use realloc()
void vector_reserve(vector_t * const v, size_t newCapacity) {
	if (newCapacity <= v->capacity) return;
	newCapacity = ((newCapacity >> 5) + 1) << 5;	// round up
	byte * const newdata = newarray(byte, v->elemSize * newCapacity);
	if (v->data) {
		memcpy(newdata, v->data, v->elemSize * MIN(newCapacity, v->capacity));
		deleteprim(v->data);
	}
	v->data = newdata;
	v->capacity = newCapacity;
}

byte * vector_back(vector_t * const v) {
	if (!v->data) fail("vector_back: vector has no data");
	if (!v->size) fail("vector_back: vector has no size");
	return v->data + v->elemSize * (v->size - 1);
}
