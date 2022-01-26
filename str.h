#pragma once

//str.h
//(TODO string?)

#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <stdio.h>	//printf
#include <assert.h>	//assert

//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning

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
	(println_move, void, (str_t * s)))
STRUCT(str,
	(str_vtable_t const *, v, 0),		// vtable
	(size_t, len, 1),			// len is the blob length (not including the \0 at the end)
	(char *, ptr, 2)			// ptr is len+1 in size for strlen strs
)

//_destroy is in-place / object dtors
//deallocate members
//c++ eqiv of destructor : str::~str
void str_destroy(str_t * const s) {
	if (!s) return;
	deleteprim(s->ptr);
	s->ptr = NULL;
}

MAKE_DEFAULTS(str, ALLOC, FREE)

void str_init(str_t * const s) {
	s->len = 0;
	s->ptr = NULL;
}

//init an empty string with the specified size
void str_init_size(str_t * const s, size_t size) {
	s->len = size;
	s->ptr = newarray(char, size+1);
}

//init a string (heap alloc) from a c-string
void str_init_c(str_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our str_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//allocate members
//c++ eqiv of constructor: str::str
//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void str_init_fmt(str_t * const s, char const * const fmt, ...) {
	assert(s);
	/*should we assert the mem in is dirty, or should we assert it is initialized and cleared?*/
	assert(!s->ptr);

	va_list args, copy;
	va_start(args, fmt);
	va_copy(copy, args);
	int len = vsnprintf(NULL, 0, fmt, args);
	if (len < 0) fail_cstr("vsnprintf failed");
	va_end(copy);

	char * const ptr = newarray(char, len + 1);
	vsnprintf(ptr, len + 1, fmt, copy);
	va_end(args);

	s->len = len;
	s->ptr = ptr;
}

str_t * str_cat(str_t const * const a, str_t const * const b) {
	str_t * const s = newobj(str, );
	s->len = a->len + b->len;	//because for now len includes the null term
	assert(!s->ptr);
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
