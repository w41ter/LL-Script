#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <cstdint>

namespace script
{
    typedef int32_t Pointer;

    enum Tag {
        TAG_NOT = 0,
        TAG_FIXNUM = 1,
        TAG_REAL = 2,
        TAG_SPEC = 3, // NULL 'C'

        TAG_CHAR = 3,
        TAG_NIL = 7,

        TAG_SHIFT = 2,
        TAG_MASK = 3,
        TAG_SPEC_MASK = 0xf,
        TAG_SPEC_SHIFT = 4,
    };
    
    bool IsNil(Pointer p);
    bool IsReal(Pointer p);
    bool IsChar(Pointer p);
    bool IsSpecal(Pointer p);
    bool IsFixnum(Pointer p);
    bool IsTagging(Pointer p);

    int GetFixnum(Pointer p);
    char GetChar(Pointer p);
    float GetReal(Pointer p);

    Pointer SetChar(char c);
    Pointer MakeFixnum(int value);
    Pointer MakeReal(float value);
    Pointer Nil();

    // 8bits
    enum Type {
        TYPE_PAIR = 0,
        TYPE_STRING = 1,
        TYPE_CLOSURE = 2,
        TYPE_ARRAY = 3,
        TYPE_BUILD_IN_CLOSURE = 4,
    };

    // common property of heap object
    // obType_ is the type of object
#define HEAP_OBJECT_HEAD    \
    int8_t obType_;         \
    int8_t resv1_;          \
    int8_t resv2_;          \
    int8_t resv3_

    typedef struct {
        HEAP_OBJECT_HEAD;
    } CommonType;

    typedef struct {
        HEAP_OBJECT_HEAD;
        Pointer car, cdr;
    } Pair;

    // make pair from alloced memory.
    Pointer MakePair(Pointer self, Pointer car, Pointer cdr);

    bool IsPair(Pointer p);

    Pointer *Car(Pointer self);
    Pointer *Cdr(Pointer self);

    typedef struct
    {
        HEAP_OBJECT_HEAD;
        size_t length_;
        char str_[];
    } String;

#define STRING_SIZE(length) (sizeof(String) + ((length) + 1) * sizeof(char))

    /* make string from exist str and memory. */
    Pointer MakeString(Pointer self, const char *from, size_t length);

    bool IsString(Pointer p);

    const char *GetString(Pointer p);

    typedef struct
    {
        HEAP_OBJECT_HEAD;
        size_t position_;
        size_t need_;
        size_t length_;
        Pointer params[];
    } Closure;

#define CLOSURE_SIZE(length) (sizeof(Closure) + (length) * sizeof(Pointer))

    Pointer MakeClosure(Pointer self, size_t position, size_t length, size_t need);
    Pointer *ClosureParams(Pointer self);
    size_t ClosureNeed(Pointer self);
    size_t ClosureLength(Pointer self);
    size_t ClosurePosition(Pointer self);
    bool IsClosure(Pointer p);

    typedef struct
    {
        HEAP_OBJECT_HEAD;
        size_t position_;
        size_t need_;
        size_t length_;
        Pointer params[];
    } BuildInClosure;

    Pointer MakeBuildInClosure(Pointer self, size_t position, size_t length, size_t need);
    Pointer *BuildInClosureParams(Pointer self);
    size_t BuildInClosureNeed(Pointer self);
    size_t BuildInClosureLength(Pointer self);
    size_t BuildInClosureIndex(Pointer self);
    bool IsBuildInClosure(Pointer p);

    typedef struct
    {
        HEAP_OBJECT_HEAD;
        size_t length_;
        Pointer data[];
    } RArray;

#define ARRAY_SIZE(length) (sizeof(RArray) + (length) * sizeof(Pointer))

    Pointer MakeArray(Pointer self, size_t length_);
    Pointer *ArrayData(Pointer self);
    size_t ArrayLength(Pointer self);
    bool IsArray(Pointer self);

    size_t SizeOfObject(Pointer p);

}

#endif // !__RUNTIME_H__
