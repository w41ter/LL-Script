#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t Object;

int GetFixnum(Object self);
float GetReal(Object self);

Object CreateFixnum(int value);
Object CreateReal(float value);
Object CreateNil();

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

Object CreateUserClosure(Object self, void *content, size_t total);
void UserClosurePushParam(Object self, Object param);
Object *UserClosureParams(Object self);
Object UserClosureParamAt(Object self, size_t index);
size_t UserClosureHold(Object self);
size_t UserClosureTotal(Object self);
void *UserClosureContent(Object slef);

Object CreateHashTable(Object self);

size_t SizeOfHashTable();
size_t SizeOfUserClosure(size_t total);
size_t SizeOfClosure(size_t total);
size_t SizeOfString(size_t length);

int ToFixnum(Object self);

bool ToLogicValue(Object self);
bool IsCallable(Object self);

bool IsHashTable(Object self);
bool IsUserClosure(Object self);
bool IsClosure(Object self);
bool IsString(Object self);

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
