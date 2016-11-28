# 介绍

simple script, just for fun.

详细设计访问[介绍](https://github.com/thinkermao/LL-Script/blob/master/introduce.md)。

## Syntax

[Here](https://github.com/thinkermao/LL-Script/blob/master/grammar.md)

## Intro

基础类型: integer, *float*, string, hash table, lambda

基础语句: if, while, return, break, continue

工作流程图:

```
lexer -> parser -> SSA IR -> reg alloc -> code gen -> OPCODE -> vm
```

## Example

```
function fib(n) {
  if (n <= 2) {
    return 1;
  }
  else {
    return fib(n - 1) + fib(n - 2);
  }
}

let n = to_integer(input("please input value:"));
let result = fib(n);

output("the result of fib(", n, ") is :", result, "\n");

```

## FFI

提供了与C语言交互的简单方式：

```
// on `lib.cpp`

Object lib_output(VMState * state, size_t paramsNums);

static Lib libs[] = {
	{ "output", lib_output },
  // ...
	{ nullptr, nullptr }
};

Object lib_output(VMState * state, size_t paramsNums)
{
	VMScene *scene = state->getScene();
	for (size_t idx = scene->paramsStack.size() - paramsNums;
		idx < scene->paramsStack.size(); ++idx) {
		Object arg = scene->paramsStack[idx];
		DumpObject(arg);
	}
	return CreateNil();
}
```
