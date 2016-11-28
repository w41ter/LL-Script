## 概述

首先看整体流程：输入代码、输出opcode并执行

```
Lexer(source code) => token stream
Parser(token stream) => SSA (static single assginment form)
CodeGen(SSA) => opcode
VirtualMachine(opcode) => result
```

这里直接从源码构造中间代码，省去了语法树部分。另外Parser后应该为优化阶段，其输出为优化后的 SSA IR。

## 词法分析、语法分析

词法分析部分是简单的 ad-hoc 生成 token stream。Parser 按照 LL(1) 文法分析，并直接生成 SSA IR。

Script 只有最基本的作用域，没有块级作用域，整个函数中的变量在函数任意位置均可访问。`FunctionScope` 表示一个函数作用域，其原型如下：

```
struct FunctionScope {
    typedef std::unordered_map<std::string, unsigned> Symbols;

    // symbol type
    enum { None, Define, Let };

    FunctionScope()
        : cfg_(nullptr)
        , block_(nullptr)
    {}

    CFG *cfg_;
    Symbols symbolTable_;
    BasicBlock *block_;
    std::unordered_set<std::string> captures_;
};
```

`cfg` 表示函数对应 SSA 的控制流程图，主要用于提供源代码直接到IR的转换，`block_` 表示当前翻译过程中源代码对应IR所在的基本块。`Symbols` 使用 `unordered_map` 来表示符号容器，其中`first`表示符号名，`second`表示符号类别，由`enum { None, Define, Let };`决定。`symbolTable_` 就是主要的符号表，函数作用域中定义的符号均记录在此。`captures_`表示函数的捕获(c++名词)列表，记录在此，给翻译收尾工作提供必要的信息。

在翻译过程中，主要用到查找符号原型如下：

```
bool tryToCatchID(std::string &name);
bool tryToCatchID(scope_iterator iter, std::string &name);
```

首先通过原型1在当前作用域中查找，如果找到返回true，如果没有，则从里向外递归地遍历函数作用域栈。找到后，如果为外部符号，则加入捕获列表，否则返回false。

## SSA 形式中间代码生成

从源代码直接生成 SSA 形式代码可以参考论文：

> Simple and Efficient Construction of Static Single Assignment Form 
> Matthias Braun1, Sebastian Buchwald1, Sebastian Hack2, Roland Leißa2, Christoph Mallon2, and Andreas Zwinkau1

Script 中涉及到的代码主要在 Parser 分析过程中以及 CFG 中，CFG 部分内容如下：

```
class CFG
{
public:
    // ....
    // SSA form construction.
    void sealOthersBlock();
    void sealBlock(BasicBlock *block);
    void saveVariableDef(std::string name, BasicBlock *block, Value *value);
    Value *readVariableDef(std::string name, BasicBlock *block);
protected:
    // SSA
    Value *readVariableRecurisive(std::string name, BasicBlock *block);
    Value *addPhiOperands(std::string name, Phi *phi);
    Value *tryRemoveTrivialPhi(const std::string &name, Phi *phi);
    // ....
};
```

这部分代码详细使用请看论文或[博客](http://www.hashcoding.net/2016/08/18/Simple-and-Direction-SSA-Constriruction-Algorithm/)。

### SSA IR 

SSA IR 的 use-def chain 主要参考 LLVM 2.0 源码。SSA IR 中基础数据结构有 `Use`、`Value`、`User`，每一条 `Instruction` 既是 `Value` 又是 `User`，`User` 和 `Value` 通过 `Use` 关联起来。一些列指令组成 `BasicBlock`，一些列 `BasicBlock` 组成 `CFG`。`IRContext`包装了`Instruction`创建的工作。每一个函数由一个 `CFG` 组成，所有函数一起组成 `IRModule`。也就是说 Parser 输出为 `IRModule`。 

### 函数处理

生成 SSA IR 部分内容并不复杂，除了有关函数、Lambda部分。Script中无论函数定义可以看作如下替换(步骤1)：

```
function bar() {}
=>
function $lambda_1_bar() {}
define bar = $lambda_1_bar;
```

因为闭包对于普通变量通过拷贝、对于复合变量通过引用(实际实现为指针)实现。所以可以通过类似于C++`std::bind`的机制实现 closure(步骤2)。

```
let a = 10;
define call = lambda(x) {
    return x + a;
}
=>
let call_tmp = lambda(a, x) ....
 define call = call_tmp(a);
```

不过这样处理就会涉及到函数中访问其自己名字的问题，或者说叫访问lambda函数自己。这个问题可以通过 Y combinator 解决。Script 中使用的办法比较简单(步骤3)：

```
let fib = lambda(n) {
    if (n < 2)	return 0;
    return fib(n-1) + fib(n-2);
}
=>
let tmp = lambda(fib, n) {
    fib = fib(fib);
    if (n < 2) return 0;
    return fib(n-1) + fib(n-2);
}
let fib = tmp(fib);
```

有了上述基础，就可以看相关代码：

```
Value *Parser::parseFunctionCommon(const std::string &name)
{
    // create function and generate parallel invoke.
    // for module require, need insert file name
    IRFunction *function = module_.createFunction(
        Combinator(lexer_.filename(), name));
    pushFunctionScopeAndInit(function);

    Strings params;
    getFunctionParamsAndBody(params, function);

    // save current captures.
    std::vector<std::string> prototype;
    getFunctionPrototype(name, prototype, params);
    function->setParams(std::move(prototype));

    std::unordered_set<std::string> captures;
    std::swap(captures, scope->captures_);
    if (captures.find(name) != captures.end())
        dealRecursiveDecl(name);
    popFunctionScope(function);

    // create closure for function.
    return createClosureForFunction(name, captures);
}
```

`parseFunctionCommon` 首先分析函数参数和函数体，然后获取函数原型(步骤2)，根据函数是否递归调用自身，调用`dealRecursiveDecl`(步骤3)，最后为函数创建闭包(步骤1)。

在函数原型部分，Script 将自身的名字放到 capture 列表的最后部分位置，这样使得处理递归调用变得简单。

## 目标代码生成

代码生成流程如下:

```
void CodeGen::runOnFunction(IRFunction *func)
{
    std::list<LiveInterval> intervals;
    {
        LiveIntervalAnalysis analysis;
        analysis.runOnFunction(func);
        analysis.swapIntervals(intervals);
    }

    SimpleRegisterAllocation RA(255, intervals);
    RA.runOnFunction(func);
    PhiElimination PE;
    PE.runOnFunction(func);

    numOfRegister = RA.totalRegister();
    genFunction(func);
}
```

首先是分析变量活性区间，然后通过活性区间进行寄存器分配，最后消除 Phi 结点，此时 IR 退出 SSA 形式。然后在这个基础上以函数为单位进行代码生成。

CodeGen 目标为opcode，而opcode相关操作包装在OPBuilder中。具体逻辑比较简单，请直接看源码。

### 活性区间计算与寄存器分配

活性区间及寄存器分配以:

> Linear Scan Register Allocation for the Java HotSpot™ Client Compiler - Christian Wimmer

论文为基础，详细逻辑可以参考该论文。

活性区间计算涉及到函数如下：

```
void buildIntervals(IRFunction *func);
void computeLocalLiveSet(IRFunction *func);
void computeGlobalLiveSet(IRFunction *func);
```

按照论文，首先计算 `BasicBlock` 顺序并编号，然后依次计算本地活性集和全局活性集，最后才建立活性区间。

```
void LiveIntervalAnalysis::runOnFunction(IRFunction *func)
{
    func->computeBlockOrder();
    computeLocalLiveSet(func);
    computeGlobalLiveSet(func);
    buildIntervals(func);
}
```

寄存器分配部分，原先的想法是使用线性扫描寄存器分配方法，在实现过程中，部分细节不知道怎么处理，所以就使用了比较简单的分配方式，将所有变量放到栈上，所有临时变量才分配寄存器，由 VM 约定最多255个寄存器。在 `SimpleRegisterAllocation` 中具体操作如下：

```
void initAndSortIntervals();
void assignRegister(IRFunction *func);
void allocateNewRegister(LiveInterval &interval);
void expiredOldIntervals(unsigned current, ActiveSet &active, ActiveSet &inactive);
bool tryToAllocateRegister(LiveInterval &interval, ActiveSet &active, ActiveSet &inactive);
```

具体实现过程，参考[博客](http://www.hashcoding.net/2016/09/06/Linear-Scan-Register-Allocation/);

## Runtime

虚拟机的数据对象采用 Tagging 标记方式，整个 Runtime 以 C 语言为主实现，封装后供 C++ 使用。对象基本结构如下：

```
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
	TypeArray = TypeString + 1,
	TypeClosure = TypeString + 2,
	TypeUserFunc = TypeString + 3,
	TypeHashNode = TypeString + 4,
	TypeUserData = TypeString + 5,
	TypeHashTable = TypeString + 6,
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
```

`Tag` 用于标记简单对象类型，复合对象继承自 `CommonObject`，使用 `obType` 表示具体对象类型即 `Type` 中对应的类型。

以String为例 ：

```
typedef struct
{
	HEAP_OBJECT_HEAD;
	size_t length;
	char str[];
} String;
```

任何一个 String 对象长度为 `sizeof(String) + String.length`。所有数据结构中，最为复杂的是 Hash 表的设计。

```
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
```

Hash 是 Hash 表的数据结构，其中数据存放在 HashNodeList 部分，这样设计是用于后面的 Expand 以及 Shrink 操作。这里说明 hash 表 set 操作：

```

static void HashSet(Object self, uintptr_t key, Object value)
{
	assert(IsFixnum(value));

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
```

首先计算hash对象所在 HashNodeList 中的 slot，如果当前位置已经被占用，则计算下一个 slot 所在位置。找到位置后，根据是否 nil 判断是否删除结点。

```
Object HashFind(Object self, Object key)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self; 
	uint32_t hash_key = HashKey(key);
	uintptr_t index = hash_key % hash->capacity;

	HashNodeList *list = hash->content;
	while (list->content[index].key != hash_key
		&& !IsNil(list->content[index].value)) {
		if (IsUndef(list->content[index].value))
			break;
		index = HashNextIndex(index, list->capacity);
	}

	if (IsNil(list->content[index].value))
		return CreateUndef();
	else
		return list->content[index].value;
}
```

find 操作查找部分与 set 类似，最后直接返回对象值。实际上对外暴露的接口是：

```
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
```

这里对 set 后的表进行判断，并根据结果伸展或者收缩表。伸展表时，目标容积为原容积的 3/2，收缩时，目标容积为原容积的 2/3。

需要注意的是 Runtime 中申请内存也必须在 Runtime 中，保证 GC 能够访问到该内存。所以 Runtime 设计中出现的内存分配必须使用 GC 提供的接口。另外，如果分配过程中发生了 GC，也需要保证所有指针均指向正确的位置。

```
// 
// just expand content.
static void HashExpand(Object self)
{
	assert(IsHash(self));
	Hash *hash = (Hash*)self;
	size_t future_capacity = HashExpandSize(hash->capacity);

	// before gc, save it as global object
	GlobalObjectBuffer = &self;
	HashNodeList *cap = (HashNodeList*)HashNewNodeList(future_capacity);
	GlobalObjectBuffer = NULL;

	HashRehash(self, cap);
}
```

因此，在 Runtime 中提供了一个特殊的对象 `GlobalObjectBuffer`来保证内存指针始终指向正确的位置。

## Virtual Machine & GC 

```
typedef int8_t Byte;

enum Opcode {
    OK_Goto = 0,    // goto lable@_addr

    // operator
    // single 
    OK_Not,         // temp = !temp
    // binary 
    OK_Add,         // temp = temp + temp
    OK_Sub,         // temp = temp - temp
    OK_Mul,         // temp = temp * temp
    OK_Div,         // temp = temp / temp
    // relop
    OK_Great,       // temp = temp > temp
    OK_GreatThan,   // temp = temp >= temp
    OK_Less,        // temp = temp < temp
    OK_LessThan,    // temp = temp <= temp
    OK_Equal,       // temp = temp == temp
    OK_NotEqual,    // temp = temp != temp

    // move
    OK_MoveS,       // temp = string index
    OK_MoveI,       // temp = constant
    OK_MoveF,		// temp = float
    OK_MoveN,		// temp = null
    OK_Move,        // temp = temp

    // memory
    OK_Load,        // load id to temp
    OK_Index,       // load id [ temp ] to temp
    OK_Store,       // store id from temp
    OK_SetIndex,      // store id [ temp ] from temp

    // condition jmp
    OK_If,          // if temp goto label

    // call 
    OK_Param,       // push temp
    OK_Call,        // temp = call Label in num params
    OK_TailCall,
    OK_Return,      // return temp
    OK_NewHash,		// tmp = new hash
    OK_NewClosure,	// tmp = new string(idx)
    OK_UserClosure, // tmp = new user closure
    OK_Halt,        // stop
};
```

opcode 设计如上，VM 对其加以解释执行。

VM 中有三个重要的数据结构，VMFrame、VMScene、VMState，VMFrame 是运行时的一个调用栈帧，保存了当前所在位置的运行时数据。VMScene是一个运行场景，每一个程序一个运行场景，当前场景运行不会影响到后续场景的运行。VMState是opcode解释器，绑定VMScene运行。在同一时刻，只能有一个VMState运行，因为需要涉及到Runtime中的设计。当然这中设计导致了这种缺陷，但是目前是最简单的实现方式。

GC 采用了semi-space swap算法，

```
struct Semispace
{
    Object allocateMemory(size_t size);
    bool contains(Object obj);
    void reset();
};

class GarbageCollector
{
public:
    Object allocate(size_t size);

    void bindReference(std::function<VariableReference> call);
    void bindGlobals(std::function<GloablVariable> call);
    void processReference(Object *slot);

private:
    void garbageCollect();

    void swapSpace();
    void cleanSpace(Semispace *space);

    bool isForwarded(Object obj);
    void forwardTo(Object obj, Object new_addr);
    Object forwardee(Object obj);

    Object swap(Object obj, size_t size);

    Semispace *from_space_;
    Semispace *to_space_;
};
```

主要数据结构如上。其中有两个回掉函数，其中`GloablVariable`用于与VM沟通，获取根结点数据;`VariableReference`用于与Runtime沟通，处理变量引用，这样将GC与VM和Runtime解耦，以后修改GC更为方便。GC详细介绍可以看[文章](http://hllvm.group.iteye.com/group/topic/39376)。

在 buildin.cpp 中提供了 Runtime 涉及到的操作和 GC 耦合部分内容:

```
void ProcessGlobals(void *scene)
{
	using script::VMFrame;
	using script::VMScene;
	using script::GarbageCollector;

	VMScene *vmscene = static_cast<VMScene*>(scene);
	GarbageCollector *GC = &vmscene->GC;
	for (auto &frame : vmscene->frames) {
		GC->processReference(&frame.params);
		GC->processReference(&frame.registers);
	}
	for (auto &object : vmscene->paramsStack) {
		GC->processReference(&object);
	}
	if (GlobalObjectBuffer != NULL)
		GC->processReference(GlobalObjectBuffer);
}

void ProcessVariableReference(void *scene, Object *object)
{
	using script::VMScene;
	using script::GarbageCollector;

	VMScene *vmscene = static_cast<VMScene*>(scene);
	GarbageCollector *GC = &vmscene->GC;
	if (IsClosure(*object)) {
		size_t hold = ClosureHold(*object);
		Object *params = ClosureParams(*object);
		for (size_t idx = 0; idx < hold; ++idx) {
			GC->processReference(&(params[idx]));
		}
	}
	else if (IsArray(*object)) {
		size_t length = ArraySize(*object);
		Object *array = ArrayPointer(*object);
		for (size_t idx = 0; idx < length; ++idx) {
			GC->processReference(&array[idx]);
		}
	}
	else if (IsHash(*object)) {
		GC->processReference(HashNodeListGet(*object));
	}
	else if (IsHashNodeList(*object)) {
		size_t size = NodeListElementCapacity(*object);
		HashNode *nodes = HashNodeListElement(*object);
		for (size_t idx = 0; idx < size; ++idx) {
			GC->processReference(&nodes[idx].value);
		}
	}
}
```

上面代码就是用于处理 GC 的回掉工作。

在 VM 当中提供了 tailCall 指令用于尾调用(并不局限与尾递归)优化，对于那些在返回语句中调用了其他函数的调用语句均可以优化。具体VM中则是重复利用当前 VMFrame ，达到尾调用的目的。

仍然需要注意的是使用 GC 出现的具有迷惑性的问题，如果某条指令执行过程中，创建了对象，就要保证创建前后指针值的位置正确。

## FFI 支持

在Runtime和VM中提供了FFI支持：

```
///
/// user func object
///
typedef struct
{
	HEAP_OBJECT_HEAD;
	void *content;
} UserClosure;
```

该结构用于包装用户函数，然后 VM 中可以使用 callUserClosure 与用户定义函数进行交互。

这些用户定义函数需要在编译器中先进行注册后才能被编译器识别：

```
Object lib_require(VMState *state, size_t paramsNums)
{
	assert(globalReguireCallback);
	if (paramsNums != 1) {
		state->runtimeError("require only takes one parameter");
	}

	Object res = state->getScene()->paramsStack.back();
	if (IsString(res)) {
		// save it.
		std::string filename = StringGet(res); 
		VMScene *scene = state->getScene();
		unsigned resReg = static_cast<unsigned>(scene->lastValue);
		globalReguireCallback(filename.c_str(), resReg);
	}
	return CreateUndef();
}

static Lib libs[] = {
	{ "require", lib_require },
	{ nullptr, nullptr }
};

void RegisterLibrary(LibRegister lib_register)
{
	Lib *lib = libs;
	while (lib->name) {
		lib_register(lib->name, lib->closure);
		lib++;
	}
}

void RegisterRequire(RequireCallback require)
{
	globalReguireCallback = require;
}
```

这里是对脚本提供了模块加载功能，首先用用户提供自定义函数(原型与lib_require一致)，然后保存在`libs[]`中，编译器初始化时，会将具体的注册函数传递给RegisterLibrary并一一注册。

对于一个C函数的定义，在注册过程中需要向Parser和opcode中注册，方便符号查找。

