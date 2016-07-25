#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "driver.h"
#include "Parser\lexer.h"
#include "Parser\Parser.h"
#include "IR\IRGenerator.h"
#include "Runtime\VM.h"
#include "Runtime\opcode.h"
#include "IR\CodeGen.h"
#include "CompilerInstance.h"

using namespace script;

int main(int argc, char* argv[])
{
    CompilerInstance compiler;
    auto &diag = compiler.getDiagnosisConsumer();
    diag.setMode(true);

    Driver &driver = compiler.getDriver();
    if (!driver.parseArguments(argc, argv))
        return 0;

    IRModule module;
    Lexer lexer(diag);
    Parser parser(lexer, module, diag);

    try {
        lexer.setProgram(std::string(driver.filename));

        parser.parse();
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    int error = diag.errors(), warning = diag.warnings();
    std::cout << "error(" << error << "), warning(" << warning << ")" << std::endl;
    if (error) 
        return 0;
    return 0;
    // generate opcode
    script::OpcodeContext opcode;
    script::CodeGenerator codegen(opcode);
    codegen.gen(module);

    int length = 0;
    Byte *opcodes = opcode.getOpcodes(length);

    try {
        script::VirtualMachine vm;
        //buildin::BuildIn::getInstance().bindGC(vm.getGC());
        vm.excute(opcodes, length);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    getchar();
	return 0;
}