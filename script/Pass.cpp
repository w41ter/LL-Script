#include "Pass.h"

namespace script
{
	void FunctionPass::runOnFunction(IRFunction * func)
	{
		assert(func && "Must point a IRFunction!");
		assert(!becalled && " Each function pass can be used only once!");
		becalled = true;
	}
}