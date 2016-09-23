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
	TypeHashNode = 4,
	TypeArray = 5,
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
/// user func object
///
typedef struct
{
	HEAP_OBJECT_HEAD;
	void *content;
} UserClosure;

///
/// array object
///
typedef struct
{
	HEAP_OBJECT_HEAD;
	size_t length;
	Object array[];
} Array;

Object CreateUserClosure(Object self, void * func)
{
	UserClosure *this = (UserClosure*)self;
	this->obType = TypeUserFunc;
	this->content = func;
	return self;
}

void * UserClosureGet(Object self)
{
	assert(IsUserClosure(self));
	return ((UserClosure*)self)->content;
}

size_t SizeOfArray(size_t total)
{
	return sizeof(Array) + total * sizeof(Array);
}

size_t SizeOfUserClosure() 
{
	return sizeof(UserClosure);
}

size_t SizeOfClosure(size_t total) 
{
	return sizeof(Closure) + total * sizeof(Object);
}

size_t SizeOfString(size_t length) 
{
	return sizeof(String) + (length + 1) * sizeof(char);
}

int ToFixnum(Object self)
{
	if (IsReal(self))
		return GetFixnum(self);
	else if (IsString(self))
		return StringSize(self) == 0;
	else if (!IsTagging(self))
		return 1;
	else if (IsNil(self))
		return 0;
	else 
		return GetFixnum(self);
}

/**
 * True - translate self to bool and return.
 *
 * Nil - false
 * Real - true
 * Fixnum - 0 = false, otherwise 1
 * Struct - true
 */
bool ToLogicValue(Object self)
{
	if (!IsTagging(self) || IsReal(self))
		return true;
	else if (IsNil(self))
		return false;
	else
		return GetFixnum(self);
}

bool IsCallable(Object self)
{
	return IsClosure(self) || IsUserClosure(self);
}

bool IsArray(Object self)
{
	return (!IsTagging(self)
		&& ((Array*)self)->obType == TypeArray);
}

bool IsHash(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeHashTable);
}

bool IsHashNodeList(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeHashNode);
}

bool IsUserClosure(Object self)
{
	return (!IsTagging(self) 
		&& ((CommonObject*)self)->obType == TypeUserFunc);
}

bool IsClosure(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeClosure);
}

bool IsString(Object self)
{
	return (!IsTagging(self)
		&& ((CommonObject*)self)->obType == TypeString);
}

bool IsUndef(Object self)
{
	return self == 0;
}

bool IsNil(Object self)
{
	return self == TagNil;
}

bool IsReal(Object self)
{
	return (self & TagMask) == TagReal;
}

bool IsSpecal(Object self)
{
	return (self & TagMask) == TagSpec;
}

bool IsFixnum(Object self)
{
	return (self & TagMask) == TagFixnum;
}

bool IsTagging(Object self)
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

Object CreateUndef()
{
	return 0;
}

Object CreateString(Object self, const char *source, size_t length)
{
	String *this = (String *)self;
	this->obType = TypeString;
	this->length = length;
	strncpy(this->str, source, length);
	this->str[length] = '\0'; // ensure for C call
	return (Object)this;
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
	return (Object)this;
}

void ClosurePushParam(Object self, Object param)
{
	assert(IsClosure(self));
	Closure *this = (Closure *)self;
	assert(this->hold + 1 <= this->total);
	this->params[this->hold] = param;
	++this->hold;
}

Object *ClosureParams(Object self)
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

Object CreateArray(Object self, size_t length)
{
	Array *this = (Array*)self;
	this->obType = TypeArray;
	this->length = length;
	memset(this->array, 0, sizeof(Object) * length);
	return (Object)this;
}

void ArraySet(Object self, size_t idx, Object value)
{
	assert(IsArray(self));
	Array *this = (Array*)self;
	assert(this->length > idx);
	this->array[idx] = value;
}

Object ArrayGet(Object self, size_t idx)
{
	assert(IsArray(self));
	Array *this = (Array*)self;
	assert(this->length > idx);
	return this->array[idx];
}

size_t ArraySize(Object self)
{
	assert(IsArray(self));
	return ((Array*)self)->length;
}

Object *ArrayPointer(Object self)
{
	assert(IsArray(self));
	return ((Array*)self)->array;
}

typedef struct HashNodeList
{
	HEAP_OBJECT_HEAD;
	size_t capacity;
	HashNode content[0];
} HashNodeList;

typedef struct Hash
{
	HEAP_OBJECT_HEAD;
	size_t capacity;
	size_t size;
	size_t max_idx;
	HashNodeList *content;
} Hash;

#define HASH_MIN_CAPACITY           (16)

#define _HASH_SEED	(size_t)0xdeadbeef

static size_t HashValue(Object key)
{
	// hash _Keyval to size_t value one-to-one
	//return ((size_t)key ^ _HASH_SEED);
	return key;
}

static size_t HashSEQ(const unsigned char *_First, size_t _Count)
{	// FNV-1a hash function for bytes in [_First, _First + _Count)
	static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;

	size_t _Val = _FNV_offset_basis;
	for (size_t _Next = 0; _Next < _Count; ++_Next)
	{	// fold in another byte
		_Val ^= (size_t)_First[_Next];
		_Val *= _FNV_prime;
	}
	return (_Val);
}

static uintptr_t HashKey(Object key)
{
	uintptr_t hash_key;
	if (IsString(key))
		hash_key = HashSEQ(StringGet(key), StringSize(key));
	else if (IsFixnum(key))
		hash_key = GetFixnum(key);
	else
		hash_key = HashValue(key);
	return hash_key;
}

static uintptr_t HashNextIndex(uintptr_t key, size_t capacity)
{
	return ((key * 2166136261u) + 16777619u) % capacity;
}

static size_t HashExpandSize(size_t original_capacity)
{
	// expand 3/2
	return original_capacity + (original_capacity >> 1);
}															

static size_t HashShrinkSize(size_t original_capacity)
{
	// shrink 2/3
	return (size_t)((original_capacity << 1) / 3);
}

static size_t HashNodeListSize(size_t capacity)
{
	return capacity * sizeof(HashNode) + sizeof(HashNodeList);
}

static Object CreateHashNodeList(Object self, size_t capacity)
{
	HashNodeList *this = (HashNodeList*)self;
	this->obType = TypeHashNode;
	this->capacity = capacity;
	memset(this->content, CreateUndef(), sizeof(HashNode) * capacity);
	return self;
}

size_t SizeOfHashNodeList(size_t capacity)
{
	return HashNodeListSize(capacity);
}

static size_t SizeOfHash()
{
	return sizeof(Hash);
}

static size_t HashTotalSize(size_t capacity)
{
	return sizeof(Hash) + HashNodeListSize(capacity);
}

extern Object Allocate(size_t size);

//
// allocate enough capacity.
static Object HashNewNodeList(size_t capacity)
{
	Object node_list = Allocate(HashNodeListSize(capacity));
	CreateHashNodeList(node_list, capacity);
	return node_list;
}

static void HashSet(Object self, uintptr_t key, Object value)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self;
	uintptr_t index = key % hash->capacity;
	uintptr_t slot = hash->capacity;

	if (IsNil(value)) {
		value = CreateUndef();
	}

	HashNodeList *list = hash->content;
	while (list->content[index].key != key
		&& !IsNil(list->content[index].value)) {
		if (slot == hash->capacity
			&& IsUndef(list->content[index].value)) {
			slot = index;
			break;
		}
		index = HashNextIndex(index, hash->capacity);
	}

	if (list->content[index].key == key)
		slot = index;
	if (slot == hash->capacity)
		slot = index;

	if (!IsUndef(value) && (IsNil(list->content[slot].value)
		|| IsUndef(list->content[slot].value))) {
		assert(hash->size != hash->capacity);
		if (key == HashKey(CreateFixnum(hash->max_idx)))
			hash->max_idx++;
		hash->size++;
	}

	if (IsUndef(value) && !IsUndef(list->content[slot].value)
		&& !IsNil(list->content[slot].value)) {
		assert(hash->size);
		if (key == HashKey(CreateFixnum(hash->max_idx - 1)))
			hash->max_idx--;
		hash->size--;
	}

	list->content[slot].value = value;
	list->content[slot].key = key;
}

static void HashRehash(Object osrc, HashNodeList *content)
{
	assert(IsHash(osrc));

	Hash *src = (Hash*)osrc;

	// replace content and capacity
	size_t old_capacity = src->capacity;
	HashNodeList *old_list = src->content;
	src->content = content;
	src->capacity = content->capacity;
	src->size = 0;
	src->max_idx = 0;

	HashNode *ptr, *limit;
	for (ptr = old_list->content, limit = old_list->content + old_capacity;
		ptr != limit; ++ptr) {
		if (!IsUndef(ptr->value) && !IsNil(ptr->value))
			HashSet(osrc, ptr->key, ptr->value);
	}
}

// 
// just expand content.
static void HashExpand(Object self)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self;
	size_t future_capacity = HashExpandSize(hash->capacity);
	HashNodeList *cap = (HashNodeList*)HashNewNodeList(future_capacity);
	HashRehash(self, cap);
}

// 
// just shrink content.
static void HashShrink(Object self)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self;
	size_t future_capacity = HashShrinkSize(hash->capacity);
	HashNodeList *cap = (HashNodeList*)HashNewNodeList(future_capacity);
	HashRehash(self, cap);
}

static bool HashNeedExpand(Object self)
{
	assert(IsHash(self));
	size_t capacity = HashCapacity(self);
	return HashSize(self) > (capacity - (capacity >> 2));
}

static bool HashNeedShrink(Object self)
{
	assert(IsHash(self));
	size_t capacity = HashCapacity(self);
	return capacity > HASH_MIN_CAPACITY
		&& HashSize(self) < (capacity >> 1);
}

Object CreateHash()
{
	// just allocate once.
	size_t capacity = HASH_MIN_CAPACITY;
	Hash *hash = (Hash*)Allocate(HashTotalSize(capacity));
	hash->obType = TypeHashTable;
	hash->capacity = capacity;
	hash->size = 0;
	hash->max_idx = 0;
	hash->content = (HashNodeList*)((char*)hash + sizeof(Hash));
	CreateHashNodeList((Object)hash->content, capacity);
	return (Object)hash;
}

size_t HashCapacity(Object hash)
{
	assert(IsHash(hash));
	return ((Hash*)hash)->capacity;
}

size_t HashSize(Object self)
{
	assert(IsHash(self));
	return ((Hash*)self)->size;
}

HashNode *HashElement(Object self)
{
	assert(IsHash(self));
	return ((Hash*)self)->content->content;
}

Object HashFind(Object self, Object key)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self; 
	uint32_t hash_key = HashKey(key);
	uintptr_t index = hash_key & (hash->capacity - 1);

	HashNodeList *list = hash->content;
	while (list->content[index].key != hash_key
		&& !IsNil(list->content[index].value)) {
		index = HashNextIndex(index, list->capacity);
	}

	if (IsNil(list->content[index].value))
		return CreateUndef();
	else
		return list->content[index].value;
}

size_t NodeListSize(Object self)
{
	assert(IsHashNodeList(self));
	return HashNodeListSize(((HashNodeList*)self)->capacity);
}

Object * HashNodeListGet(Object self)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self;
	return (Object*)(&hash->content);
}

void HashSetAndUpdate(Object self, Object key, Object value)
{
	assert(IsHash(self));

	if (key == CreateFixnum(-1))
		key = CreateFixnum(((Hash*)self)->max_idx);
	uint32_t hash_key = HashKey(key);
	HashSet(self, hash_key, value);
	if (HashNeedExpand(self)) 
		HashExpand(self);
	else if (HashNeedShrink(self)) 
		HashShrink(self);
}

size_t SizeOfObject(Object p)
{
    if (IsTagging(p) || IsUndef(p))
        return 0;
    CommonObject *Object = (CommonObject*)p;
    switch (Object->obType)
    {
    case TypeString:
        return SizeOfString(((String*)p)->length);
    case TypeClosure:
        return SizeOfClosure(((Closure*)p)->total);
	case TypeArray:
		return SizeOfArray(((Array*)p)->length);
	case TypeHashTable:
		return SizeOfHash();
	case TypeHashNode:
		return SizeOfHashNodeList(((HashNodeList*)p)->capacity);
    }
    return 0;
}
