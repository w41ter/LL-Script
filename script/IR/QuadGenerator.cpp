#include "QuadGenerator.h"

namespace script
{
    void QuadGenerator::insertIfFalse(Value * condition, Label * label)
    {
        codes_.push_back(Quad::Create<IfFalse>(condition, label));
    }

    void QuadGenerator::insertStore(Value * id, Value * result)
    {
        codes_.push_back(Quad::Create<Store>(id, result));
    }

    void QuadGenerator::insertReturn(Value * value)
    {
        codes_.push_back(Quad::Create<Return>(value));
    }

    void QuadGenerator::insertLabel(Label * label)
    {
        codes_.push_back(label);
    }

    void QuadGenerator::insertGoto(Label * label)
    {
        codes_.push_back(Quad::Create<Label>(label));
    }

    void QuadGenerator::insertCopy(Value * source, Value * dest)
    {
        codes_.push_back(Quad::Create<Copy>(source, dest));
    }

    void QuadGenerator::insertFunction(std::string name, Label * begin, Label * end)
    {
        Function *func = Quad::Create<Function>(name, begin, end);
        codes_.push_back(func);
    }

    void QuadGenerator::insertCall(Label * begin, Value * result, int num)
    {
        codes_.push_back(Quad::Create<Call>(begin, result, num));
    }

    void QuadGenerator::insertInvoke(Value * function, Value * result, int num)
    {
        codes_.push_back(Quad::Create<Call>(function, result, num));
    }

    void QuadGenerator::insertParam(Value * value)
    {
        codes_.push_back(Quad::Create<Param>(value));
    }

    void QuadGenerator::insertSingle(unsigned op, Value * source, Value * result)
    {
        codes_.push_back(Quad::Create<Operation>(op, nullptr, source, result));
    }

    void QuadGenerator::insertOperation(unsigned op, Value * left, Value * right, Value * result)
    {
        codes_.push_back(Quad::Create<Operation>(op, left, right, result));
    }

    void QuadGenerator::insertIf(Value * condition, Label * label)
    {
        codes_.push_back(Quad::Create<If>(condition, label));
    }
}
