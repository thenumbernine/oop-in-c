#pragma once


//str.h


//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning
STRUCT(str,
	(size_t, len, 0),		// len is the blob length (not including the \0 at the end)
	(char *, ptr, 1)		// ptr is len+1 in size for strlen strs
)


//_init is for in-place init / is the ctor
void str_init_c(str_t * s, char const * cstr);
void str_init(str_t * s, char const * fmt, ...);

//_destroy is in-place / object dtors
void str_destroy(str_t * s);

//_new calls _init for heap allocated objects


// C functions
str_t * str_new(char const * fmt, ...);

//_move means free args after you're done
str_t * str_cat_move(str_t * a, str_t * b);


//safealloc.cpp


void * safealloc(size_t size) {
	void * const ptr = calloc(size, 1);
	if (!ptr) {
		fail("malloc failed for %u bytes\n", size);
		return NULL;
	}
	return ptr;
}


//str.cpp


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

//deallocate members
//c++ eqiv of destructor : str::~str
void str_destroy(str_t * const s) {
	if (!s) return;
	delete(s->ptr);
	s->ptr = NULL;
	s->len = 0;
}

MAKE_DEFAULTS(str, ALLOC, FREE, DELETE)

//str_new_c => calls str_alloc, str_init_c
//c++ equiv of "new str(cstr)"
MAKE_NEW_FOR_INIT(str, _c,
	(char const *, cstr))

//can't use DEFAULT_NEW since it uses va_list which can't be forwarded
	// can't forward va-args so copy the above body ...
	// ... so use a macro for the _init body instead
str_t * str_new(char const * const fmt, ...) {
	str_t * s = str_alloc();
	str_init_body(s);
	return s;
}

str_t * str_cat(str_t const * const a, str_t const * const b) {
	str_t * const s = str_alloc();
	s->len = a->len + b->len;	//because for now len includes the null term
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, a->ptr, a->len);
	memcpy(s->ptr + a->len, b->ptr, b->len);
	s->ptr[a->len + b->len] = '\0';
	return s;
}

MAKE_MOVE2(str, str, cat, str)	//str_cat_move from str_cat

void str_println(str_t * const s) {
	printf("%s\n", s->ptr);
}

MAKE_MOVE_VOID(str, println);	//str_println_move from str_println
