#include <string.h>
#include <assert.h>

#include "Runtime.h"

enum Tag {
	TagNot = 0,
	TagFixnum = 1,
	TagReal = 2,
	TagSpec = 3,

	TagNil = 7,

	TagShift = 2,
	TagMask = 3,

	TagSpecalMask = 0xf,
	TagSpecalShift = 4,
};

// 8bits
enum Type {
	TypeString = 0,
	TypeClosure = 1,
	TypeUserFunc = 2,
	TypeHashTable = 3,
};

// common property of heap object
// obType_ is the type of object
#define HEAP_OBJECT_HEAD   \
	int8_t obType;         \
	int8_t resv1;          \
	int8_t resv2;          \
	int8_t resv3

typedef struct {
	HEAP_OBJECT_HEAD;
} CommonObject;

///
/// String object
/// 
typedef struct
{
	HEAP_OBJECT_HEAD;
	size_t length;
	char str[];
} String;

///
/// script closure object
///
typedef struct
{
	HEAP_OBJECT_HEAD;
	void *content;
	size_t total;
	size_t hold;
	Object params[];
} Closure;

///
/// hash table object.
///
typedef struct
{
	HEAP_OBJECT_HEAD;
} HashTable;

///
/// user func object
///
typedef struct
{
	HEAP_OBJECT_HEAD;
	void *content;
	size_t total;
	size_t hold;
	Object params[];
} UserClosure;

inline size_t SizeOfHashTable() {
	return sizeof(HashTable);
}

inline size_t SizeOfUserClosure(size_t total) {
	return sizeof(Closure) + total * sizeof(Object);
}

inline size_t SizeOfClosure(size_t total) {
	return sizeof(Closure) + total * sizeof(Object);
}

inline size_t SizeOfString(size_t length) {
	return sizeof(String) + (length + 1) * sizeof(char);
}

/**
 * True - translate self to bool and return.
 *
 * Nil - false
 * Real - true
 * Fixnum - 0 = false, otherwise 1
 * Struct - true
 */
inline bool True(Object self)
{
	if (IsTagging(self) || IsReal(self))
		return true;
	else if (IsNil(self))
		return false;
	else
		return GetFixnum(self);
}

inline bool False(Object self)
{
	return !True(self);
}

inline bool IsCallable(Object self)
{
	return IsClosure(self);
}

inline bool IsHashTable(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeHashTable);
}

inline bool IsUserClosure(Object self)
{
	return (!IsTagging(self) 
		&& ((CommonObject*)self)->obType == TypeUserFunc);
}

inline bool IsClosure(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeClosure);
}

inline bool IsString(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeString);
}

inline bool IsNil(Object self)
{
	return self == TagNil;
}

inline bool IsReal(Object self)
{
	return (self & TagMask) == TagReal;
}

inline bool IsSpecal(Object self)
{
	return (self & TagMask) == TagSpec;
}

inline bool IsFixnum(Object self)
{
	return (self & TagMask) == TagFixnum;
}

inline bool IsTagging(Object self)
{
	return (self & TagMask) != TagNot;
}

int GetFixnum(Object self)
{
	assert(IsFixnum(self));
	return (int)(self >> TagShift);
}

float GetReal(Object self)
{
	assert(IsReal(self));
	int val = (int)(self >> TagShift);
	return *(float*)&val;
}

Object CreateFixnum(int value)
{
	return (value << TagShift) | TagFixnum;
}

// FIXEME: dangerous
Object CreateReal(float value)
{
	int val = *(int*)&value;
	return (val << TagShift) | TagReal;
}

Object CreateNil()
{
	return TagNil;
}

Object CreateString(Object self, const char *source, size_t length)
{
	String *this = (String *)self;
	this->obType = TypeString;
	this->length = length;
	strncpy(this->str, source, length);
	this->str[length] = '\0'; // ensure for C call
	return this;
}

const char *StringGet(Object self)
{
	assert(IsString(self));
	return ((String*)self)->str;
}

size_t StringSize(Object self)
{
	assert(IsString(self));
	return ((String*)self)->length;
}

Object CreateClosure(Object self, void *content, size_t total)
{
	assert(content);
	Closure *this = (Closure *)self;
	this->obType = TypeClosure;
	this->hold = 0;
	this->total = total;
	this->content = content;
	return this;
}

void ClosurePushParam(Object self, Object param)
{
	assert(IsClosure(self));
	Closure *this = (Closure *)self;
	assert(this->hold + 1 <= this->total);
	this->params[this->hold] = param;
	++this->hold;
}

const Object *ClosureParams(Object self)
{
	assert(IsClosure(self));
	return ((Closure*)self)->params;
}

Object ClosureParamAt(Object self, size_t index)
{
	assert(IsClosure(self));
	Closure *this = (Closure *)self;
	assert(this->total > index);
	return this->params[index];
}

size_t ClosureHold(Object self)
{
	assert(IsClosure(self));
	return ((Closure*)self)->hold;
}

size_t ClosureTotal(Object self)
{
	assert(IsClosure(self));
	return ((Closure*)self)->total;
}

void *ClosureContent(Object self)
{
	assert(IsClosure(self));
	return ((Closure*)self)->content;
}

size_t SizeOfObject(Object p)
{
    if (IsTagging(p))
        return 0;
    CommonObject *Object = (CommonObject*)p;
    switch (Object->obType)
    {
    case TypeString:
        return SizeOfString(((String*)p)->length);
    case TypeClosure:
        return SizeOfClosure(((Closure*)p)->total);
    }
    return 0;
}