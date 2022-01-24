#pragma once

//TODO put this with the rest of the DEFAULT_ macros?
//TODO this reference "str_cat_move"
//so be sure to include "str.h" if you use this.

#define DEFAULT_TOSTR(type)\
str_t * type##_tostr(\
	type##_t const * const obj\
) {\
	str_t * s = str_new_c(#type);\
	if (!obj) {\
		return str_cat_move(s, str_new_c("NULL"));\
	}\
	s = str_cat_move(s, str_new("%p={", obj));\
	/* TODO HERE FOR_EACH over the reflect fields, and then call each member's _tostr() */\
	reflect_t * endOfFields = type##_fields + numberof(type##_fields);\
	for (reflect_t * field = type##_fields; field < endOfFields; ++field) {\
		if (field > type##_fields) {\
			s = str_cat_move(s, str_new_c(", "));\
		}\
		s = str_cat_move(s, str_new("%s=", field->name));\
/*		s = str_cat_move(s, tostring(  ));*/\
	}\
	s = str_cat_move(s, str_new_c("}"));\
	return s;\
}
