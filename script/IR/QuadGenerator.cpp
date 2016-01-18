#include "QuadGenerator.h"

namespace script
{
    void QuadGenerator::insertIfFalse(Value * condition, Label * label)
    {
        codes_.push_back(Create<IfFalse>(condition, label));
    }

    void QuadGenerator::insertStore(Value * id, Value * result)
    {
        codes_.push_back(Create<Store>(id, result));
    }

    void QuadGenerator::insertLoad(Value * id, Value * result)
    {
        codes_.push_back(Create<Load>(id, result));
    }

    void QuadGenerator::insertReturn(Value * value)
    {
        codes_.push_back(Create<Return>(value));
    }

    void QuadGenerator::insertLabel(Label * label)
    {
        codes_.push_back(label);
    }

    void QuadGenerator::insertGoto(Label * label)
    {
        codes_.push_back(Create<Goto>(label));
    }

    void QuadGenerator::insertCopy(Value * source, Value * dest)
    {
        codes_.push_back(Create<Copy>(source, dest));
    }

    void QuadGenerator::insertCall(Label * begin, Value * result, int num)
    {
        codes_.push_back(Create<Call>(begin, result, num));
    }

    void QuadGenerator::insertInvoke(Value * function, Value * result, int num)
    {
        codes_.push_back(Create<Invoke>(function, result, num));
    }

    void QuadGenerator::insertParam(Value * value)
    {
        codes_.push_back(Create<Param>(value));
    }

    void QuadGenerator::insertSingle(unsigned op, Value * source, Value * result)
    {
        codes_.push_back(Create<Operation>(op, nullptr, source, result));
    }

    void QuadGenerator::insertOperation(unsigned op, Value * left, Value * right, Value * result)
    {
        codes_.push_back(Create<Operation>(op, left, right, result));
    }

    void QuadGenerator::insertIf(Value * condition, Label * label)
    {
        codes_.push_back(Create<If>(condition, label));
    }

    QuadFunction * QuadModule::createFunction(std::string name, Label * begin, Label * end)
    {
        return functions_[name] = new QuadFunction(name, begin, end);
    }

    QuadFunction * QuadModule::getFunction(std::string & name)
    {
        return functions_[name];
    }
}
