#include "Buildin.h"

static inline bool IsCalable(Object self) 
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

void ProcessGlobals(void *scene)
{
	//Frame *temp = currentFrame_;
	//while (true)
	//{
	//	for (size_t i = 0; i < temp->localSlot_.size(); ++i)
	//	{
	//		gc_.processReference(&(temp->localSlot_[i]));
	//	}
	//	temp = temp->previous_;
	//	if (temp == nullptr) break;
	//}
}

void ProcessVariableReference(Object *Object)
{
	//if (IsClosure(*Object))
	//{
	//	size_t need = ClosureNeed(*Object);
	//	Object *params = ClosureParams(*Object);
	//	for (size_t i = 0; i < need; ++i)
	//	{
	//		gc_.processReference(&(params[i]));
	//	}
	//}
}
