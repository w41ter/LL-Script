# 介绍

simple script, just for fun.

## Syntax

[Here](https://github.com/thinkermao/LL-Script/blob/master/grammar.md)

## Intro

base database: integer, *float*, string, hash table, lambda

base statement: if, while, return, break, continue

graph:

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