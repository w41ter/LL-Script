#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "VM.h"
#include "lib.h"
#include "driver.h"
#include "lexer.h"
#include "Parser.h"
#include "IRModule.h"
#include "DumpIR.h"
#include "OpBuilder.h"
#include "CodeGen.h"
#include "dumpOpcode.h"
#include "OpcodeModule.h"
#include "UnreachableBlockElimination.h"
#include "CompilerInstance.h"

using namespace script;

const char *globalMainName = "$main";

int main(int argc, char* argv[])
{
    CompilerInstance compiler;
    auto &diag = compiler.getDiagnosisConsumer();
    diag.setMode(true);

    Driver &driver = compiler.getDriver();
    if (!driver.parseArguments(argc, argv))
        return 0;

    IRModule module;
	OpcodeModule opcode; 

	Lexer lexer(diag);
	Parser parser(lexer, module, diag);

	VMState state;
	VMScene scene{ opcode };

	BindGCProcess(&scene);
	RegisterLibrary([&parser, &opcode](const char *name,
		UserDefLibClosure closure) {
		parser.registerUserClosure(name);
		opcode.pushUserClosure(name, closure);
	});



    try {
        lexer.setProgram(std::string(driver.filename));

        parser.parse();
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    int error = diag.errors(), warning = diag.warnings();
    if (error)
    {
		std::cout << "error(" << error << "), warning("
			<< warning << ")" << std::endl;
        getchar();
        return 0;
    }

#ifdef _DEBUG
	if (driver.dumpIR_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpIR dumpIR(filename += ".op_before.ir");
		dumpIR.dump(&module);
	}
#endif // _DEBUG

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

	if (driver.dumpIR_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpIR dumpIR(filename += ".ir");
		dumpIR.dump(&module);
	}
 
	CodeGen codegen(opcode);
	for (auto &func : module)
	{
#ifdef _DEBUG
		std::cout << "Code generator: "
			<< func.first << std::endl;
#endif // _DEBUG
		codegen.runOnFunction(func.second);
	}

	if (driver.dumpOpcode_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpOpcode dumpOpcode(opcode, filename += ".opcode");
		dumpOpcode.dump();
	}

	Object function = scene.GC.allocate(SizeOfClosure(0));
	auto *content = &opcode.getFunction(globalMainName);
	OPBuilder::GenHalt(*content);
	CreateClosure(function, content, 0);
	state.bindScene(&scene);
	state.call(function, 0, 0);
	state.execute();

    getchar();
	return 0;
}