#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t Object;

typedef struct HashNode
{
	uintptr_t key;
	Object value;
} HashNode;

int GetFixnum(Object self);
float GetReal(Object self);

Object CreateFixnum(int value);
Object CreateReal(float value);
Object CreateNil();
Object CreateUndef();

/* make string from exist str and memory. */
Object CreateString(Object self, const char *source, size_t length);
const char *StringGet(Object self);
size_t StringSize(Object self);

Object CreateClosure(Object self, void *content, size_t total);
void ClosurePushParam(Object self, Object param);
Object *ClosureParams(Object self);
Object ClosureParamAt(Object self, size_t index);
size_t ClosureHold(Object self);
size_t ClosureTotal(Object self);
void *ClosureContent(Object self);

Object CreateArray(Object self, size_t length);
void ArraySet(Object self, size_t idx, Object value);
Object ArrayGet(Object self, size_t idx);
size_t ArraySize(Object self);
Object *ArrayPointer(Object self);

Object CreateHash();
size_t HashCapacity(Object hash);
size_t HashSize(Object self);
Object HashFind(Object self, Object key);
size_t NodeListSize(Object self);
Object *HashNodeListGet(Object self);
HashNode *HashElement(Object self);
void HashSetAndUpdate(Object self, Object key, Object value);

Object CreateUserClosure(Object self, void *func);
void *UserClosureGet(Object self);

size_t SizeOfArray(size_t total);
size_t SizeOfUserClosure();
size_t SizeOfClosure(size_t total);
size_t SizeOfString(size_t length);

int ToFixnum(Object self);

bool ToLogicValue(Object self);
bool IsCallable(Object self);

bool IsArray(Object self);
bool IsHash(Object self);
bool IsHashNodeList(Object self);
bool IsUserClosure(Object self);
bool IsClosure(Object self);
bool IsString(Object self);

bool IsUndef(Object self);
bool IsNil(Object self);
bool IsReal(Object self);
bool IsSpecal(Object self);
bool IsFixnum(Object self);
bool IsTagging(Object self);

size_t SizeOfObject(Object self);

#ifdef __cplusplus
}
#endif 

#endif // !__RUNTIME_H__
