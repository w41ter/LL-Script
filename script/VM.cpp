#include "VM.h"

#include <string>
#include <vector>
#include <functional>
#include <cassert>
#include <string.h>

using std::vector;
using std::string;

namespace script
{
	Object add(Object lhs, Object rhs);
	Object sub(Object lhs, Object rhs);
	Object mul(Object lhs, Object rhs);
	Object div(Object lhs, Object rhs);
	Object g(Object lhs, Object rhs);
	Object gt(Object lhs, Object rhs);
	Object l(Object lhs, Object rhs);
	Object lt(Object lhs, Object rhs);
	Object et(Object lhs, Object rhs);
	Object ne(Object lhs, Object rhs);
	Object not_(Object lhs);
	Object negative(Object lhs);

    //void VirtualMachine::processGlobals()
    //{
    //    Frame *temp = currentFrame_;
    //    while (true)
    //    {
    //        for (size_t i = 0; i < temp->localSlot_.size(); ++i)
    //        {
    //            gc_.processReference(&(temp->localSlot_[i]));
    //        }
    //        temp = temp->previous_;
    //        if (temp == nullptr) break;
    //    }
    //}

    //void VirtualMachine::variableReference(Object *Object)
    //{
    //    if (IsClosure(*Object))
    //    {
    //        size_t need = ClosureNeed(*Object);
    //        Object *params = ClosureParams(*Object);
    //        for (size_t i = 0; i < need; ++i)
    //        {
    //            gc_.processReference(&(params[i]));
    //        }
    //    }
    //}

    //bool IsCalable(Object lhs)
    //{
    //    return (IsFixnum(lhs) || IsReal(lhs));
    //}

   /* Object VirtualMachine::add(Object lhs, Object rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) + GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left + right);
        }
    }

    Object VirtualMachine::sub(Object lhs, Object rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) - GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left - right);
        }
    }

    Object VirtualMachine::mul(Object lhs, Object rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) * GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left * right);
        }
    }

    Object VirtualMachine::div(Object lhs, Object rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            int right = GetFixnum(rhs);
            return MakeFixnum(GetFixnum(lhs) / right);
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            if (right == 0.0f) throw std::runtime_error("div 0 error!");
            return MakeReal(left / right);
        }
    }

    Object VirtualMachine::g(Object lhs, Object rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        return MakeFixnum(GetFixnum(lhs) > GetFixnum(rhs));
    }

    Object VirtualMachine::gt(Object lhs, Object rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        return MakeFixnum(GetFixnum(lhs) >= GetFixnum(rhs));
    }

    Object VirtualMachine::l(Object lhs, Object rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        return MakeFixnum(GetFixnum(lhs) < GetFixnum(rhs));
    }

    Object VirtualMachine::lt(Object lhs, Object rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        return MakeFixnum(GetFixnum(lhs) <= GetFixnum(rhs));
    }

    Object VirtualMachine::et(Object lhs, Object rhs)
    {
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(lhs == rhs);
        }
        else if (IsReal(lhs) && IsReal(rhs))
        {
            return MakeFixnum(lhs == rhs);
        }
        else if (IsString(lhs) && IsString(rhs))
        {
            return MakeFixnum(strcmp(GetString(lhs), GetString(rhs)) == 0);
        }
        else
        {
            return MakeFixnum(lhs == rhs);
        }
    }

    Object VirtualMachine::ne(Object lhs, Object rhs)
    {
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(lhs != rhs);
        }
        else if (IsReal(lhs) && IsReal(rhs))
        {
            return MakeFixnum(lhs != rhs);
        }
        else if (IsString(lhs) && IsString(rhs))
        {
            return MakeFixnum(strcmp(GetString(lhs), GetString(rhs)));
        }
        else
        {
            return MakeFixnum(lhs != rhs);
        }
    }

    Object VirtualMachine::not_(Object lhs)
    {
        if (!IsFixnum(lhs))
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
        return MakeFixnum(!GetFixnum(lhs));
    }

    Object VirtualMachine::negative(Object lhs)
    {
        if (IsFixnum(lhs))
        {
            return MakeFixnum(-GetFixnum(lhs));
        }
        else if (IsReal(lhs))
        {
            return MakeReal(-GetReal(lhs));
        }
        else
        {
            throw std::runtime_error("���Ͳ�ƥ��");
        }
    }

    NewVM::NewVM(int stackSize)
    {
        stackSize_ = stackSize;
    }
*/
	VMState::VMState()
	{

	}

	void VMState::execute(VMScene & scene)
	{
		if (scene.frames.size() == 0)
			return;

		currentScene = &scene;

		bool runState = true;
		while (runState) {
			topFrame = currentScene->frames.back();
			auto &ip = topFrame->ip;
			switch (topFrame->content->codes[ip++])
			{
			case OK_Add:
			case OK_Sub:
			case OK_Div:
			case OK_Mul:
			case OK_Great:
			case OK_GreatThan:
			case OK_Less:
			case OK_LessThan:
			case OK_NotEqual:
			case OK_Equal:
				executeBinary(ip, topFrame->content->codes[ip - 1]);
				break;
			case OK_Not:
				executeNotOP(ip);
				break;
			case OK_Call:
				executeCall(ip);
				break;
			case OK_TailCall:
				executeTailCall(ip);
				break;
			case OK_Goto:
				executeGoto(ip);
				break;
			case OK_If:
				executeIf(ip);
				break;
			case OK_Return:
				executeReturn(ip);
				break;
			case OK_Load:
				executeLoad(ip);
				break;
			case OK_Store:
				executeStore(ip);
				break;
			case OK_Index:
				executeIndex(ip);
				break;
			case OK_SetIndex:
				executeSetIndex(ip);
				break;
			case OK_Move:
				executeMove(ip);
				break;
			case OK_MoveF:
				executeMoveF(ip);
				break;
			case OK_MoveI:
				executeMoveI(ip);
				break;
			case OK_MoveS:
				executeMoveS(ip);
				break;
			case OK_MoveN:
				executeMoveN(ip);
				break;
			case OK_Halt:
				runState = false;
				break;
			case OK_Param:
				executeParam(ip);
				break;
			case OK_NewClosure:
				executeNewClosure(ip);
				break;
			case OK_NewHash:
				executeNewHash(ip);
				break;
			default:
				break;
			}
		}
	}

	void VMState::runtimeError(const char * str)
	{
	}

	void VMState::executeBinary(size_t & ip, unsigned op)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object left = topFrame->getRegVal(opcode[ip++]);
		Object right = topFrame->getRegVal(opcode[ip++]);

		Object calRes = 0;
		switch (op)
		{
		case OK_Add: calRes = add(left, right); break;
		case OK_Sub: calRes = sub(left, right); break;
		case OK_Div: calRes = div(left, right); break;
		case OK_Mul: calRes = mul(left, right); break;
		case OK_Great: calRes = g(left, right); break;
		case OK_GreatThan: calRes = gt(left, right); break;
		case OK_Less: calRes = l(left, right); break;
		case OK_LessThan: calRes = lt(left, right); break;
		case OK_NotEqual: calRes = ne(left, right); break;
		case OK_Equal: calRes = et(left, right); break;
		}
		topFrame->setRegVal(result, calRes);
	}

	void VMState::executeNotOP(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object val = topFrame->getRegVal(opcode[ip++]);
		Object res = not_(val);
		topFrame->setRegVal(result, res);
	}

	void VMState::executeMove(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = topFrame->getRegVal(opcode[ip++]);
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveF(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = CreateReal(getFloat(ip));
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveI(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = CreateFixnum(getInteger(ip));
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveS(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t stringIndex = getInteger(ip);
		const std::string &string = 
			currentScene->module.getString(stringIndex);
		Object str = currentScene->GC.allocate(
			SizeOfString(string.size()));
		CreateString(str, string.c_str(), string.size());
		topFrame->setRegVal(result, str);
	}

	void VMState::executeMoveN(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		topFrame->setRegVal(result, CreateNil());
	}

	void VMState::executeCall(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned resultReg = opcode[ip++];
		Object func = topFrame->getRegVal(opcode[ip++]);
		int32_t paramsNums = getInteger(ip);
		if (!IsCallable(func)) 
			runtimeError("try to invoke incallable object");
		int32_t totalParams = ClosureTotal(func);
		int32_t hasParams = ClosureHold(func);
		if (hasParams + paramsNums > totalParams)
			runtimeError("too many params");
		if (hasParams + paramsNums == totalParams) {
			OpcodeFunction *content = 
				static_cast<OpcodeFunction*>(ClosureContent(func));
			VMFrame *newFrame = new VMFrame {
				content->numOfregisters, content->paramSize,
				topFrame, resultReg, content
			};
			size_t currentParamSize = currentScene->paramsStack.size();
			for (size_t i = 0; i < hasParams; ++i) {
				newFrame->params[i] = ClosureParamAt(func, i);
			}
			for (size_t i = paramsNums; i > 0; --i) {
				size_t from = currentParamSize - i;
				size_t current = hasParams + from;
				newFrame->params[current] = currentScene->paramsStack[from];
			}
			currentScene->frames.push_back(newFrame);
		}
		else {
			// TODO: new closure.
			size_t currentParamSize = currentScene->paramsStack.size();
			for (size_t i = paramsNums; i > 0; --i) {
				size_t from = currentParamSize - i;
				size_t current = hasParams + paramsNums - i;
				ClosurePushParam(func, currentScene->paramsStack[from]);
			}
		}
	}

	void VMState::executeTailCall(size_t & ip)
	{
	}

	void VMState::executeGoto(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned offset = getInteger(ip);
		ip = offset;
	}

	void VMState::executeIf(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object cond = topFrame->getRegVal(opcode[ip++]);
		unsigned offset = getInteger(ip);
		if (True(cond))
			ip = offset;
	}

	void VMState::executeReturn(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object val = topFrame->getRegVal(opcode[ip++]);
		currentScene->frames.pop_back();
		currentScene->frames.back()->setRegVal(topFrame->resReg, val);
		delete topFrame;
	}

	void VMState::executeLoad(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t slot = getInteger(ip);
		Object val = topFrame->getParamVal(slot);
		topFrame->setRegVal(result, val);
	}

	void VMState::executeStore(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		assert(0);
	}

	void VMState::executeIndex(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object table = topFrame->getRegVal(opcode[ip++]);
		Object index = topFrame->getRegVal(opcode[ip++]);
		// TODO: get index and set.
	}

	void VMState::executeSetIndex(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object table = topFrame->getRegVal(opcode[ip++]);
		Object index = topFrame->getRegVal(opcode[ip++]);
		Object data = topFrame->getRegVal(opcode[ip++]);
		// TODO: set hash.
	}

	void VMState::executeParam(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object val = topFrame->getRegVal(opcode[ip++]);
		currentScene->paramsStack.push_back(val);
	}

	void VMState::executeNewClosure(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		// TODO: make closure
		topFrame->setRegVal(result, CreateNil());
	}

	void VMState::executeNewHash(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object hash = currentScene->GC.allocate(SizeOfHashTable());
		topFrame->setRegVal(result, hash);
	}

	int32_t VMState::getInteger(size_t & ip)
	{
		int32_t result = 0;
		auto &opcode = topFrame->content->codes;
		for (int i = 0; i < 4; ++i)
		{
			result <<= 8; 
			result |= (unsigned char)opcode[ip++];
		}
		return result;
	}

	float VMState::getFloat(size_t & ip)
	{
		int ival = getInteger(ip), *iptr = &ival;
		double *fptr = reinterpret_cast<double*>(iptr);
		return *fptr;
	}
}