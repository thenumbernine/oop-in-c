#pragma once


//object.h

typedef struct object_s object_t;

//all "object subclasses" should have these matching methods
typedef struct {
	object_t * (*alloc)();
	void (*free)(object_t *);
	void (*destroy)(object_t *);
} object_vtable_t;

//all "object subclasses" should have these matching fields
STRUCT(object,
	(object_vtable_t*, v, 0))


//TODO a function + macro that calls
//but if this is a macro then "obj" gets re-evaluated ...
//and if this is a function then "func" needs extra qualifiers, and "obj" needs corret type casting
// (not to mention the problems of __VA_ARGS__)
#define CALL(obj, func, ...) \
obj->v->func(obj, __VA_ARGS__)


//str.h


typedef struct str_s str_t;

typedef struct {
	str_t * (*alloc)();
	void (*free)(str_t *);
	void (*destroy)(str_t *);
	//how about function prototype standards?
	void (*init)(str_t *, char const * const fmt, ...);		//default ctor
	void (*init_c)(str_t *, char const * const cstr);
	void (*init_size)(str_t *, size_t size);
	str_t * (*cat)(str_t const * a, str_t const * b);
	str_t * (*cat_move)(str_t * a, str_t * b);
	void (*println)(str_t * const s);
	void (*println_move)(str_t * s);
} str_vtable_t;
extern str_vtable_t str_vtable;


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
STRUCT(str,
	(str_vtable_t*, v, 0),		// vtable
	(size_t, len, 1),			// len is the blob length (not including the \0 at the end)
	(char *, ptr, 2)			// ptr is len+1 in size for strlen strs
)

//can't forward va-args, so ... 
//  says: https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
#define str_init_body(s) {\
	assert(s);\
	/*should we assert the mem in is dirty, or should we assert it is initialized and cleared?*/\
	assert(!s->ptr);\
\
	va_list args, copy;\
	va_start(args, fmt);\
	va_copy(copy, args);\
	int len = vsnprintf(NULL, 0, fmt, args);\
	if (len < 0) fail_cstr("vsnprintf failed");\
	va_end(copy);\
\
	char * const ptr = newarray(char, len + 1);\
	vsnprintf(ptr, len + 1, fmt, copy);\
	va_end(args);\
\
	s->len = len;\
	s->ptr = ptr;\
}

//_init is for in-place init / is the ctor
//allocate members
//c++ eqiv of constructor: str::str
void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//allocate members
//c++ eqiv of constructor: str::str
//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init(str_t * const s, char const * const fmt, ...) {
	str_init_body(s);
}

//_destroy is in-place / object dtors
//deallocate members
//c++ eqiv of destructor : str::~str
void str_destroy(str_t * const s) {
	if (!s) return;
	delete(s->ptr);
	s->ptr = NULL;
}

MAKE_DEFAULTS(str, ALLOC, FREE, DELETE)

//str_new_c => calls str_alloc, str_init_c
//c++ equiv of "new str(cstr)"
MAKE_NEW_FOR_INIT(str, _c,
	(char const *, cstr))

//_new calls _init for heap allocated objects
#if 1
//can't use DEFAULT_NEW since it uses va_list which can't be forwarded
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
str_t * str_new(char const * const fmt, ...) {
	str_t * s = str_alloc();
	s->v = &str_vtable;
	str_init_body(s);
	return s;
}
#else
// C vararg messes up our arg forwarding 
// so does the macro being called "_body" but I guess we can get around that
MAKE_NEW_FOR_INIT(str, EMPTY,
    (char const * const, fmt COMMA ...))
#endif

#if 0
void str_init_empty(str_t * const s) {
	s->len = 0;
	s->ptr = NULL;
}
MAKE_NEW_FOR_INIT(str, _empty)
#endif

void str_init_size(str_t * const s, size_t size) {
	s->len = size;
	s->ptr = newarray(char, size+1);
}
MAKE_NEW_FOR_INIT(str, _size, (size_t, size))

str_t * str_cat(str_t const * const a, str_t const * const b) {
	//TODO here str_new_SOMETHING
#if 1
	str_t * const s = str_alloc();
	s->v = &str_vtable;
#else
	str_t * const s = str_init_empty();
#endif
	s->len = a->len + b->len;	//because for now len includes the null term
	
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, a->ptr, a->len);
	memcpy(s->ptr + a->len, b->ptr, b->len);
	s->ptr[a->len + b->len] = '\0';
	return s;
}

//_move means free args after you're done
MAKE_MOVE2(str_t *, str, cat, str)	//str_cat_move from str_cat

void str_println(str_t * const s) {
	printf("%s\n", s->ptr);
}

MAKE_MOVE_VOID(str, println);	//str_println_move from str_println

str_vtable_t str_vtable = {
	.alloc = str_alloc,
	.free = str_free,
	.destroy = str_destroy,
	.init = str_init,
	.init_c = str_init_c,
	.init_size = str_init_size,
	.cat = str_cat,
	.cat_move = str_cat_move,
	.println = str_println,
	.println_move = str_println_move,
};
