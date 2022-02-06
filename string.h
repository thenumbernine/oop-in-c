#pragma once

#include <stdarg.h>	//va_start, va_end, vsnprintf
#include <string.h>	//strlen
#include <stdio.h>	//printf
#include <assert.h>	//assert

//combo of c and c++ strs: \0 terms and non-incl .len field at the beginning

//#define CLASS_string_super object
#define CLASS_string_fields (\
	(size_t, len), /* len is the blob length (not including the \0 at the end) */\
	(charp_t, ptr)  /* ptr is len+1 in size for strlen strs */\
)
#define CLASS_string_methods (\
	(alloc, string_t *, ()),\
	(free, void, (string_t *)),\
	(destroy, void, (string_t *)),\
	(init, void, (string_t *)),\
	(init_size, void, (string_t *, size_t size)),\
	(init_c, void, (string_t *, char const * const cstr)),\
	(init_fmt, void, (string_t *, char const * const fmt, ...)),\
	(cat, string_t *, (string_t const * a, string_t const * b)),\
	(cat_move, string_t *, (string_t * a, string_t * b)),\
	(println, void, (string_t * const s)),\
	(println_move, void, (string_t * s))\
)
CLASS(string)

//_destroy is in-place / object dtors
//deallocate members
//c++ eqiv of destructor : string::~string
void string_destroy(string_t * const s) {
	if (!s) return;
	deleteprim(s->ptr);
	s->ptr = NULL;
}

MAKE_DEFAULTS(string, ALLOC, FREE)

void string_init(string_t * const s) {
	s->len = 0;
	s->ptr = NULL;
}

//init an empty string with the specified size
void string_init_size(string_t * const s, size_t size) {
	s->len = size;
	s->ptr = newarray(char, size+1);
}

//init a string (heap alloc) from a c-string
void string_init_c(string_t * const s, char const * const cstr) {
	s->len = strlen(cstr);	//plus 1 so our string_t can be a cstr and a c++ string
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, cstr, s->len + 1);
}

//allocate members
//c++ eqiv of constructor: string::string
//https://codereview.stackexchange.com/questions/156504/implementing-printf-to-a-string-by-calling-vsnprintf-twice
void string_init_fmt(string_t * const s, char const * const fmt, ...) {
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

string_t * string_cat(string_t const * const a, string_t const * const b) {
	string_t * const s = newobj(string, );
	s->len = a->len + b->len;	//because for now len includes the null term
	assert(!s->ptr);
	s->ptr = newarray(char, s->len + 1);
	memcpy(s->ptr, a->ptr, a->len);
	memcpy(s->ptr + a->len, b->ptr, b->len);
	s->ptr[a->len + b->len] = '\0';
	return s;
}

//_move means free args after you're done
MAKE_MOVE2(string_t *, string, cat, string)	//string_cat_move from string_cat

void string_println(string_t * const s) {
	printf("%s\n", s->ptr);
}

MAKE_MOVE_VOID(string, println);	//string_println_move from string_println

//headers at the top of struct.h
MAKE_TOSTRING_FOR_FMT(byte, "%c")
MAKE_TOSTRING_FOR_FMT(char, "%c")
MAKE_TOSTRING_FOR_FMT(short, "%hd")
MAKE_TOSTRING_FOR_FMT(int, "%d")
MAKE_TOSTRING_FOR_FMT(size_t, "%zu")
MAKE_TOSTRING_FOR_FMT(intptr_t, "%zd")	//PRIxPTR
MAKE_TOSTRING_FOR_FMT(uintptr_t, "%zu")	//PRIxPTR
MAKE_TOSTRING_FOR_FMT(int8_t, "%hhd")
MAKE_TOSTRING_FOR_FMT(uint8_t, "%hhu")
MAKE_TOSTRING_FOR_FMT(int16_t, "%hd")
MAKE_TOSTRING_FOR_FMT(uint16_t, "%hu")
MAKE_TOSTRING_FOR_FMT(int32_t, "%d")
MAKE_TOSTRING_FOR_FMT(uint32_t, "%u")
MAKE_TOSTRING_FOR_FMT(int64_t, "%ld")
MAKE_TOSTRING_FOR_FMT(uint64_t, "%lu")
MAKE_TOSTRING_FOR_FMT(float, "%d")
MAKE_TOSTRING_FOR_FMT(double, "%d")

MAKE_TOSTRING_FOR_PTRTYPE(charp_t)
MAKE_TOSTRING_FOR_PTRTYPE(bytep_t)
MAKE_TOSTRING_FOR_PTRTYPE(voidp_t)

MAKE_TOSTRING_FOR_ADDR(vtable)	//generic vtable
MAKE_TOSTRING_FOR_ADDR(func)	//generic member function type
