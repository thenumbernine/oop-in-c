#pragma once

/*
TRY {
	do something
	THROW(1)
} CATCH(1) {
} ENDTRY
*/

#define TRY\
{\
	jmp_buf buf;/* TODO static vector jmp_buf stack so THROW can work in functions outside the TRY/ENDTRY scope */\
	int val = setjmp(buf);\
	switch (val) {\
	case 0:

#define ENDTRY\
		break;\
		default:\
			fail("unhandled exception %d", val);\
		break;\
	}\
}

//TODO macros to associate exception enums/ptrs with exception classnames, so we can throw objects.  too bad setjmp & longjmp don't use intptr_t's ...
#define THROW(option)\
	longjmp(buf, option);

#define CATCH(option)
	break;\
	case option:
		
