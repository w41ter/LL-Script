# 介绍

simple script.

## Syntax

[EBNF](https://github.com/thinkermao/LL-Script/blob/master/grammar.md) 点开查看语法文件。

## Updata:
	
功能：

- 基础数据类型 integer, float, character, string, array([]), function
- 支持基本语句 if, while, return, break, continue
- 尾递归优化，异常处理，柯里化函数
- VM, GC

2016/3/13：

- 准备添加tuple、map,用list替代array
- 准备添加syntax-define功能
- 准备添加 call/cc 

## Example

```
define fib = function(n) {
  let fib_iner = function(func, num) {
    if (num <= 2) return 1;
    return func(func, num-1) + func(func, num-2);
  };
  return fib_iner(fib_iner, n);
};
let value = input();
let result = fib(value);
output("the result of fib(");output(value);output(") is : "); output(result);output("\n");
```