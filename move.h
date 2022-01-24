#pragma once

#define MAKE_MOVE(returnType, objType, funcName)\
returnType##_t * objType##_##funcName##_move(objType##_t * const obj) {\
	returnType##_t * const result = objType##_##funcName(obj);\
	/*_move behavior: delete incoming objects*/\
	objType##_delete(obj);\
	return result;\
}

#define MAKE_MOVE2(returnType, objType, funcName, obj2Type)\
returnType##_t * objType##_##funcName##_move(objType##_t * const obj, obj2Type##_t * const obj2) {\
	returnType##_t * const result = objType##_##funcName(obj, obj2);\
	/*_move behavior: delete incoming objects*/\
	objType##_delete(obj);\
	obj2Type##_delete(obj2);\
	return result;\
}

#define MAKE_MOVE_VOID(objType, funcName)\
void objType##_##funcName##_move(objType##_t * const obj) {\
	objType##_##funcName(obj);\
	objType##_delete(obj);\
}
