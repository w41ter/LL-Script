# 介绍

为大创项目下的一个子项目，单独开发。本意是做成一个DSL，权当练手。

## 语法

	标识符：a-zA-Z_(a-zA-Z_0-9)*
	字符串："(*)"
	字面常量：0-9(.0-9)
	操作符：= != == <= >= < > | & + - * / % !
	
	其中字符串只支持 = != == +，其余可计算量均为double
	
	expression:
		表达式由可运算量加符号组成，可运算量为字面常量，字符串，变量，函数返回值组成。其中如果为赋值语句，那么 expression 返回等号左边变量左值。
		
	statements:
		变量声明："var" * 变量名 * ( "=" expression ) ";"
	其中如果有赋值语句，则会被解释为 * 变量名 * = expression; 变量作用域一共为两层，外部一层，函数体内一层。任何变量都需要使用var申明，但没有限制声明位置，及你可以先使用再声明。
		
		if statement: "if" "(" expression ")" "{" statements "}" ["else" "{" statements "}")]
	其中的 expression 返回为真则执行紧接着的 statements ，否则执行 else 紧接着的 statements
	
		while statement: "while" "(" expression ")" "{" statements "}"
	若 expression 的值为真，继续执行 statement 否则，结束当前 statement。
	
		break statement: "break" ";"
	用于跳出 while 循环， 如果上层没有 while 则出错。
	
		return statement: "return" expression ";"
	return 只能在函数体内调用，否则将出错。expression 的值将作为函数返回值
	
	function definatin:
		"function" * 函数名 * "(" params_list ")" "{" statements "}"
	函数名不能与最外层其他函数或变量重名。函数无需指定返回值，如果没有任何return语句，则返回 0 。
	
		params_list: * 参数名 * ("," * 参数名 *)
		其中的参数名可以任意指定，同变量处理，所以函数体内变量声明时不能使用参数名。
		
		function call: * 函数名 * "(" * 变量名 * ("," * 变量名 *))
		函数的参数必须为变量，变量数需与对应的参数数一致。其中，插件函数中的参数可以指定不限制长度，如print。
		
	plugin：
		用于与C++进行交互。目前提供了print和input函数。
		
		print：用法同普通函数一样，不限制参数长度，作用是将参数值在同一行输出并换行。
		
		input：将输入变量返回给调用者。输入变量如果是字符串则返回字符串，否则返回double。
		
	example: 
	
		function flb(n) {
			var temp1 = n-1;
			var temp2 = n-2;
			
			if (temp1 & temp2) {
				return flb(temp1) + flb(temp2);
			}
			
			return 1;
		}

		var p = "Input the Numer";
		print(p);
		p = input();
		p = flb(p);
		print(p);
		
## Updata:
	
	2015/04/29：
	修改了部分命名规则；
	将部分臃肿代码精简化；
	修改了变量是否定义的验证方式，使得效率提升近5倍。
		
	2015/5/1
	对部分内容进行了重构，简化代码结构。
	
## TODO

	添加数组功能
	添加类
	添加闭包
	
	将解释语法书的方式转换为操作码
	
	
	