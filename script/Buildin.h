#pragma once

#include "Runtime.h"

#ifdef __cplusplus
extern "C" {
#endif 

Object Add		(Object LHS, Object RHS);
Object Sub		(Object LHS, Object RHS);
Object Mul		(Object LHS, Object RHS);
Object Div		(Object LHS, Object RHS);
Object Great	(Object LHS, Object RHS);
Object Less		(Object LHS, Object RHS);
Object NotGreat	(Object LHS, Object RHS);
Object NotLess	(Object LHS, Object RHS);
Object Equal	(Object LHS, Object RHS);
Object NotEqual	(Object LHS, Object RHS);
Object Not		(Object LHS);

#ifdef __cplusplus
}
#endif 