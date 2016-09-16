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
const Object *ClosureParams(Object self);
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

inline size_t SizeOfHashTable();
inline size_t SizeOfUserClosure(size_t total);
inline size_t SizeOfClosure(size_t total);
inline size_t SizeOfString(size_t length);

inline bool True(Object self);
inline bool False(Object self);
inline bool IsCallable(Object self);

inline bool IsHashTable(Object self);
inline bool IsUserClosure(Object self);
inline bool IsClosure(Object self);
inline bool IsString(Object self);

inline bool IsNil(Object self);
inline bool IsReal(Object self);
inline bool IsSpecal(Object self);
inline bool IsFixnum(Object self);
inline bool IsTagging(Object self);

size_t SizeOfObject(Object self);

#ifdef __cplusplus
}
#endif 

#endif // !__RUNTIME_H__
