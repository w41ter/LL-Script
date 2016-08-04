#include <cstring>

#include "Runtime.h"

namespace script
{
    bool IsNil(Pointer p)
    {
        return (p & TAG_SPEC_MASK) == TAG_NIL;
    }

    bool IsReal(Pointer p)
    {
        return (p & TAG_MASK) == TAG_REAL;
    }

    bool IsChar(Pointer p)
    {
        return (p & TAG_SPEC_MASK) == TAG_CHAR;
    }

    bool IsSpecal(Pointer p)
    {
        return (p & TAG_MASK) == TAG_SPEC;
    }

    bool IsFixnum(Pointer p)
    {
        return (p & TAG_MASK) == TAG_FIXNUM;
    }

    bool IsTagging(Pointer p)
    {
        return (p & TAG_MASK);
    }

    int GetFixnum(Pointer p)
    {
        return (p >> TAG_SHIFT);
    }

    char GetChar(Pointer p)
    {
        return (p >> TAG_SPEC_MASK);
    }

    float GetReal(Pointer p)
    {
        int ival = (p >> TAG_SHIFT), *iptr = &ival;
        return *reinterpret_cast<float*>(iptr);
    }
    
    Pointer SetChar(char c)
    {
        return (c << TAG_SPEC_SHIFT) | TAG_CHAR;
    }

    Pointer MakeFixnum(int value)
    {
        return ((value << TAG_SHIFT) | TAG_FIXNUM);
    }

    Pointer MakeReal(float value)
    {
        float *fptr = &value;
        int *iptr = reinterpret_cast<int*>(fptr), ival = *iptr;
        return ((ival << TAG_SHIFT) | TAG_REAL);
    }

    Pointer Nil()
    {
        return TAG_NIL;
    }

    // make pair from alloced memory.
    Pointer MakePair(Pointer self, Pointer car, Pointer cdr)
    {
        ((Pair *)self)->obType_ = (int8_t)TYPE_PAIR;
        ((Pair *)self)->car = car;
        ((Pair *)self)->cdr = cdr;
        return self;
    }

    bool IsPair(Pointer p)
    {
        return (!IsTagging(p) && ((Pair *)p)->obType_ == (Pointer)TYPE_PAIR);
    }

    Pointer *Car(Pointer self)
    {
        return &((Pair *)self)->car;
    }

    Pointer *Cdr(Pointer self)
    {
        return &((Pair *)self)->cdr;
    }

    /* make string from exist str and memory. */
    Pointer MakeString(Pointer self, const char *from, size_t length)
    {
        ((String *)self)->obType_ = (Pointer)TYPE_STRING;
        ((String *)self)->length_ = length;
        strncpy(((String *)self)->str_, from, length);
        ((String *)self)->str_[length] = '\0';      // ensure for C call
        return self;
    }

    bool IsString(Pointer p)
    {
        return (!IsTagging(p) && ((String *)p)->obType_ == (Pointer)TYPE_STRING);
    }

    const char *GetString(Pointer p)
    {
        return ((String *)p)->str_;
    }

    Pointer MakeClosure(Pointer self, size_t position, size_t length, size_t need)
    {
        ((Closure *)self)->obType_ = (Pointer)TYPE_CLOSURE;
        ((Closure *)self)->position_ = position;
        ((Closure *)self)->length_ = length;
        ((Closure *)self)->need_ = need;
        return self;
    }

    Pointer * ClosureParams(Pointer self)
    {
        return ((Closure *)self)->params;
    }

    size_t ClosurePosition(Pointer self)
    {
        return ((Closure *)self)->position_;
    }

    size_t ClosureNeed(Pointer self)
    {
        return ((Closure *)self)->need_;
    }

    size_t ClosureLength(Pointer self)
    {
        return ((Closure *)self)->length_;
    }

    bool IsClosure(Pointer p)
    {
        return (!IsTagging(p) && ((String *)p)->obType_ == (Pointer)TYPE_CLOSURE);
    }

    Pointer MakeBuildInClosure(Pointer self, size_t position, size_t length, size_t need)
    {
        ((Closure *)self)->obType_ = (Pointer)TYPE_BUILD_IN_CLOSURE;
        ((Closure *)self)->position_ = position;
        ((Closure *)self)->length_ = length;
        ((Closure *)self)->need_ = need;
        return self;
    }

    Pointer * BuildInClosureParams(Pointer self)
    {
        return ((Closure *)self)->params;
    }

    size_t BuildInClosureIndex(Pointer self)
    {
        return ((Closure *)self)->position_;
    }

    size_t BuildInClosureNeed(Pointer self)
    {
        return ((Closure *)self)->need_;
    }

    size_t BuildInClosureLength(Pointer self)
    {
        return ((Closure *)self)->length_;
    }

    bool IsBuildInClosure(Pointer p)
    {
        return (!IsTagging(p) && ((String *)p)->obType_ == (Pointer)TYPE_BUILD_IN_CLOSURE);
    }

    Pointer MakeArray(Pointer self, size_t length_)
    {
        ((RArray *)self)->obType_ = (Pointer)TYPE_ARRAY;
        ((RArray *)self)->length_ = length_;
        return self;
    }

    Pointer * ArrayData(Pointer self)
    {
        return ((RArray *)self)->data;
    }

    size_t ArrayLength(Pointer self)
    {
        return ((RArray *)self)->length_;
    }

    bool IsArray(Pointer self)
    {
        return (!IsTagging(self) && ((RArray *)self)->obType_ == (Pointer)TYPE_ARRAY);
    }

    size_t SizeOfObject(Pointer p)
    {
        if (IsTagging(p))
            return 0;
        CommonType *pointer = (CommonType*)p;
        switch (pointer->obType_)
        {
        case Type::TYPE_PAIR:
            return sizeof(Pair);
        case Type::TYPE_STRING:
            return sizeof(String) + ((String*)p)->length_;
        case Type::TYPE_CLOSURE:
            return sizeof(Closure) + ((Closure*)p)->length_;
        case Type::TYPE_ARRAY:
            return sizeof(RArray) + ((RArray*)p)->length_;
        }
        return 0;
    }

}