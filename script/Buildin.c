#include "Buildin.h"

static inline IsCalable(Object self) 
{
	return IsFixnum(self);
}

Object Add(Object LHS, Object RHS)
{
	if (IsCalable(LHS) && IsCalable(RHS)) {
		return CreateFixnum(GetFixnum(LHS) + GetFixnum(RHS));
	}
	return CreateNil();
}

Object Sub(Object LHS, Object RHS)
{
	if (IsCalable(LHS) && IsCalable(RHS)) {
		return CreateFixnum(GetFixnum(LHS) - GetFixnum(RHS));
	}
	return CreateNil();
}

Object Mul(Object LHS, Object RHS)
{
	if (IsCalable(LHS) && IsCalable(RHS)) {
		return CreateFixnum(GetFixnum(LHS) * GetFixnum(RHS));
	}
	return CreateNil();
}

Object Div(Object LHS, Object RHS)
{
	if (IsCalable(LHS) && IsCalable(RHS)) {
		int right = GetFixnum(RHS);
		if (right == 0)
			return CreateNil();
		return CreateFixnum(GetFixnum(LHS) / GetFixnum(RHS));
	}
	return CreateNil();
}

Object Great(Object LHS, Object RHS)
{
	return CreateNil();
}

Object Less(Object LHS, Object RHS)
{
	return CreateNil();
}

Object NotGreat(Object LHS, Object RHS)
{
	return CreateNil();
}

Object NotLess(Object LHS, Object RHS)
{
	return CreateNil();
}

Object Equal(Object LHS, Object RHS)
{
	return CreateNil();
}

Object NotEqual(Object LHS, Object RHS)
{
	return CreateNil();
}

Object Not(Object LHS)
{
	return CreateFixnum(!ToLogicValue(LHS));
}
