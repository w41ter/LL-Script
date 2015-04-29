# LL-Script

大创项目中的一个子项目，为rpg游戏设计的DSL。功能简单，练手之作。

## syntax

definition:
	
	var identifier;
	var identifier = expression;
	
	function identifier(params) {
		statements;
	}
	
expression:

	op: = | &  + - * / %
	identifier(number,string) op identifier(number,string,function_call)
	
statements:

	if (expression) {
		statements;
	} else {
		statements;
	}
	
	while (expression) {
		statements;
	}
	
	return expression;
	
	break;
	
	expression;
	
funtion_call:
	
	function_name(identifier1, identifier2);
	
	break 跳出 While
	return 只能在函数中调用
	添加了 input 和 print 插件：
		print 打印参数，参数不限制长度，并在最后加上回车换行。
		input 返回从缓冲区读取的数据
		
simple:
	
	// 计算斐波拉契数列
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
	

## 更新：
	
	2015/04/29：
	修改了部分命名规则；
	将部分臃肿代码精简化；
	修改了变量是否定义的验证方式，使得效率提升近百倍。
	
	