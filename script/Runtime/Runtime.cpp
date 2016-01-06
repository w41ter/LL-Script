#include <cstring>

#include "Runtime.h"

namespace script
{
    inline bool IsNil(Pointer p)
    {
        return (p & TAG_SPEC_MASK) == TAG_NIL;
    }

    inline bool IsReal(Pointer p)
    {
        return (p & TAG_MASK) == TAG_REAL;
    }

    inline bool IsChar(Pointer p)
    {
        return (p & TAG_SPEC_MASK) == TAG_CHAR;
    }

    inline bool IsSpecal(Pointer p)
    {
        return (p & TAG_MASK) == TAG_SPEC;
    }

    inline bool IsFixnum(Pointer p)
    {
        return (p & TAG_MASK) == TAG_FIXNUM;
    }

    inline bool IsTagging(Pointer p)
    {
        return (p & TAG_MASK);
    }

    inline int GetFixnum(Pointer p)
    {
        return (p >> TAG_SHIFT);
    }

    inline char GetChar(Pointer p)
    {
        return (p >> TAG_SPEC_MASK);
    }

    inline float GetReal(Pointer p)
    {
        return (p >> TAG_SHIFT);
    }
    
    inline Pointer SetChar(char c)
    {
        return (c << TAG_SPEC_SHIFT) | TAG_CHAR;
    }

    inline Pointer MakeFixnum(int value)
    {
        return ((value << TAG_SHIFT) | TAG_FIXNUM);
    }

    inline Pointer MakeReal(float value)
    {
        return ((((int)value) << TAG_SHIFT) | TAG_REAL);
    }

    inline Pointer Nil()
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

    inline bool IsPair(Pointer p)
    {
        return (!IsTagging(p) || ((Pair *)p)->obType_ == (Pointer)TYPE_PAIR);
    }

    inline Pointer *Car(Pointer self)
    {
        return &((Pair *)self)->car;
    }

    inline Pointer *Cdr(Pointer self)
    {
        return &((Pair *)self)->cdr;
    }

    /* make string from exist str and memory. */
    Pointer MakeString(Pointer self, const char *from, size_t length)
    {
        ((String *)self)->obType_ = (Pointer)TYPE_STRING;
        ((String *)self)->length_ = length;
        strncpy(((String *)self)->str_, from, length);
        return self;
    }

    inline bool IsString(Pointer p)
    {
        return (!IsTagging(p) || ((String *)p)->obType_ == (Pointer)TYPE_STRING);
    }

    inline const char *GetString(Pointer p)
    {
        return ((String *)p)->str_;
    }

    Pointer MakeClosure(Pointer self, size_t length)
    {
        ((Closure *)self)->obType_ = (Pointer)TYPE_CLOSURE;
        ((Closure *)self)->length_ = length;
        return self;
    }

    Pointer * ClosureParams(Pointer self)
    {
        return ((Closure *)self)->params;
    }

    bool IsClosure(Pointer p)
    {
        return (!IsTagging(p) || ((String *)p)->obType_ == (Pointer)TYPE_CLOSURE);
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
        }
        return 0;
    }
}