#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <cstdlib>
#include <ctime>

#include "VM.h"
#include "lib.h"
#include "driver.h"
#include "lexer.h"
#include "Parser.h"
#include "IRModule.h"
#include "DumpIR.h"
#include "CodeGen.h"
#include "dumpOpcode.h"
#include "OpcodeModule.h"
#include "UnreachableBlockElimination.h"
#include "CompilerInstance.h"

using namespace script;

const char *globalMainName = "$main";

void DumpIRToFile(
	CompilerInstance &compiler, 
	IRModule &module,
	const char *str)
{
	auto &driver = compiler.getDriver();
	if (driver.dumpIR_)
	{
		std::string filename = str;
		filename.resize(filename.find_last_of('.'));
		DumpIR dumpIR(filename += ".ir");
		dumpIR.dump(&module);
	}
}

void Optimizer(
	CompilerInstance &compiler,
	IRModule &module)
{
	auto &driver = compiler.getDriver();
	if (driver.optimized_)
	{
		UnreachableBlockElimination UBElim;
		for (auto &func : module)
		{
#ifdef _DEBUG
			std::cout << "Eliminate unreachable block: "
				<< func.first << std::endl;
#endif // _DEBUG
			UBElim.runOnFunction(func.second);
		}
	}
}

void DumpOpcodeToFile(
	CompilerInstance &compiler, 
	OpcodeModule &opcode)
{
	auto &driver = compiler.getDriver();
	if (driver.dumpOpcode_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpOpcode dumpOpcode(opcode, filename += ".opcode");
		dumpOpcode.dump();
	}
}

void CodeGenerator(
	IRModule &module, 
	OpcodeModule &opcode)
{
	CodeGen codegen(opcode);
	for (auto &func : module)
	{
#ifdef _DEBUG
		std::cout << "Code generator: "
			<< func.first << std::endl;
#endif // _DEBUG
		codegen.runOnFunction(func.second);
	}
}

void ExecuteScriptEntry(
	VMState &state,
	VMScene &scene,
	const char *filename,
	unsigned resReg)
{
	std::string mainName = filename;
	mainName.resize(mainName.find_last_of('.'));
	mainName += "\\";
	mainName += globalMainName;

	Object function = scene.GC.allocate(SizeOfClosure(0));
	auto *content = &scene.module.getFunction(mainName);
	assert(content && "logic error");
	//OPBuilder::GenHalt(*content);
	CreateClosure(function, content, 0);
	
	state.bindScene(&scene);
	state.call(function, 0, resReg);

	try {
		state.execute();
	}
	catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}
}

void parseProgram(
	CompilerInstance &compiler,
	OpcodeModule opcode, 
	IRModule &module, 
	const char *filename) 
{
	auto &diag = compiler.getDiagnosisConsumer();

	Lexer lexer(diag);
	Parser parser(lexer, module, diag);

	RegisterLibrary([&parser](const char *name,
		UserDefLibClosure closure) {
		parser.registerUserClosure(name);
	});

	try {
		lexer.setProgram(filename);
		parser.parse();
	}
	catch (std::runtime_error &e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}

	int error = diag.errors(), warning = diag.warnings();
	if (error)
	{
		std::cout << "error(" << error << "), warning("
			<< warning << ")" << std::endl;
		getchar();
		exit(0);
	}
}

int main(int argc, char* argv[])
{
    CompilerInstance compiler;
	auto &diag = compiler.getDiagnosisConsumer();
	diag.setMode(true);

    Driver &driver = compiler.getDriver();
    if (!driver.parseArguments(argc, argv))
        return 0;

	srand(time(NULL));

	OpcodeModule opcode;
	VMState state;
	VMScene scene{ opcode };

	BindGCProcess(&scene);

	RegisterLibrary([&opcode](const char *name,
		UserDefLibClosure closure) {
		opcode.pushUserClosure(name, closure);
	});

	auto execute = [&compiler, &opcode, &state, &scene]
		(const char *name, unsigned resReg) {
		IRModule module;
		parseProgram(compiler, opcode, module, name);
		Optimizer(compiler, module);
		DumpIRToFile(compiler, module, name);
		CodeGenerator(module, opcode);
		DumpOpcodeToFile(compiler, opcode);
		ExecuteScriptEntry(state, scene, name, resReg);
	};

	// for require
	RegisterRequire(execute);

	execute(driver.filename, 0);

	return 0;
}