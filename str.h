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

#if 1

typedef str_t * (str_alloc_t)();
typedef void (str_free_t)(str_t *);
typedef void (str_destroy_t)(str_t *);
typedef void (str_init_t)(str_t *);
typedef void (str_init_size_t)(str_t *, size_t size);
typedef void (str_init_c_t)(str_t *, char const * const cstr);
typedef void (str_init_fmt_t)(str_t *, char const * const fmt, ...);		//default ctor
typedef str_t * (str_cat_t)(str_t const * a, str_t const * b);
typedef str_t * (str_cat_move_t)(str_t * a, str_t * b);
typedef void (str_println_t)(str_t * const s);
typedef void (str_println_move_t)(str_t * s);

str_alloc_t str_alloc;
str_free_t str_free;
str_destroy_t str_destroy;
str_init_t str_init;
str_init_size_t str_init_size;
str_init_c_t str_init_c;
str_init_fmt_t str_init_fmt;
str_cat_t str_cat;
str_cat_move_t str_cat_move;
str_println_t str_println;
str_println_move_t str_println_move;

STRUCT(str_vtable,
	(str_alloc_t*, alloc, 0),
	(str_free_t*, free, 1),
	(str_destroy_t*, destroy, 2),
	(str_init_t*, init, 3),
	// how about function prototype standards?
	(str_init_size_t*, init_size, 4),
	(str_init_c_t*, init_c, 5),
	(str_init_fmt_t*, init_fmt, 6),
	(str_cat_t*, cat, 7),
	(str_cat_move_t*, cat_move, 8),
	(str_println_t*, println, 9),
	(str_println_move_t*, println_move, 10)
)

str_vtable_t str_vtable = {
	.alloc = str_alloc,
	.free = str_free,
	.destroy = str_destroy,
	.init = str_init,
	.init_size = str_init_size,
	.init_c = str_init_c,
	.init_fmt = str_init_fmt,
	.cat = str_cat,
	.cat_move = str_cat_move,
	.println = str_println,
	.println_move = str_println_move,
};

#else

/*
make the str_vtable with fields of function pointers to our functions 
TODO write the prototypes of each of the functions
TODO set up the vtable here too based on the prototypes
*/
VTABLE(str,
	(alloc, str_t *, ()),
	(free, void, (str_t *)),
	(destroy, void, (str_t *)),
	(init, void, (str_t *)),
	(init_size, void, (str_t *, size_t size)),
	(init_c, void, (str_t *, char const * const cstr)),
	(init_fmt, void, (str_t *, char const * const fmt, ...)),
	(cat, str_t *, (str_t const * a, str_t const * b)),
	(cat_move, str_t *, (str_t * a, str_t * b)),
	(println, void, (str_t * const s)),
	(println_move, void, (str_t * s)),

#endif


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
STRUCT(str,
	(str_vtable_t*, v, 0),		// vtable
	(size_t, len, 1),			// len is the blob length (not including the \0 at the end)
	(char *, ptr, 2)			// ptr is len+1 in size for strlen strs
)

//_destroy is in-place / object dtors
//deallocate members
//c++ eqiv of destructor : str::~str
void str_destroy(str_t * const s) {
	if (!s) return;
	delete(s->ptr);
	s->ptr = NULL;
}

MAKE_DEFAULTS(str, ALLOC, FREE, DELETE)

void str_init(str_t * const s) {
	s->len = 0;
	s->ptr = NULL;
}
MAKE_NEW_FOR_INIT_NOARGS(str, )	//TODO make this work with MAKE_NEW_FOR_INIT with no args

//init an empty string with the specified size
void str_init_size(str_t * const s, size_t size) {
	s->len = size;
	s->ptr = newarray(char, size+1);
}
MAKE_NEW_FOR_INIT(str, _size, (size_t, size))

//init a string (heap alloc) from a c-string
void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}
MAKE_NEW_FOR_INIT(str, _c, (char const *, cstr))

//can't forward va-args, so ... 
//  says: https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
#define str_init_fmt_body(s) {\
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

//allocate members
//c++ eqiv of constructor: str::str
//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init_fmt(str_t * const s, char const * const fmt, ...) {
	str_init_fmt_body(s);
}

//_new calls _init for heap allocated objects
#if 1
//can't use DEFAULT_NEW since it uses va_list which can't be forwarded
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
str_t * str_new_fmt(char const * const fmt, ...) {
	str_t * s = str_alloc();
	s->v = &str_vtable;
	str_init_fmt_body(s);
	return s;
}
#else
// C vararg messes up our arg forwarding 
// so does the macro being called "_body" but I guess we can get around that
MAKE_NEW_FOR_INIT(str, _fmt,
    (char const * const, fmt COMMA ...))
#endif

str_t * str_cat(str_t const * const a, str_t const * const b) {
	str_t * const s = str_new();
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
