#pragma once

//TODO how to do templates ...
#define CLASS_vector_fields (\
	(size_t, size),\
	(size_t, capacity),\
	(bytep_t, data),\
	(size_t, elemSize)\
)
#define CLASS_vector_methods (\
	(alloc, vector_t *, ()),\
	(free, void, (vector_t *)),\
	(destroy, void, (vector_t *)),\
	(init, void, (vector_t *)),\
	(init_elemSize, void, (vector_t *, size_t elemSize)),\
	(init_elemSize_len, void, (vector_t *, size_t elemSize, size_t size)),\
	(push_back, void, (vector_t *, bytep_t const * what)),\
	(pop_back, void, (vector_t *, bytep_t const * what)),\
	(resize, void, (vector_t *, size_t newSize)),\
	(size, size_t, (vector_t const *)),/* getter */\
	(capacity, size_t, (vector_t const *)),/* getter */\
	(data, bytep_t, (vector_t const *)),/* getter */\
	(reserve, void, (vector_t *, size_t newCapacity)),\
	(shrink_to_fit, void, (vector_t *))\
)
CLASS(vector)
