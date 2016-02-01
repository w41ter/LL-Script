#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "driver.h"
#include "Parser\lexer.h"
#include "Parser\Parser.h"
#include "Semantic\Analysis.h"
#include "Semantic\dumpAST.h"
#include "Semantic\Translator.h"
#include "IR\QuadGenerator.h"
#include "IR\Quad.h"
#include "IR\dumpQuad.h"
#include "IR\dumpCFG.h"
#include "IR\CodeGen.h"
#include "Runtime\opcode.h"
#include "Runtime\dumpOpcode.h"

int main(int argc, char* argv[])
{
    script::Driver &driver = script::Driver::instance();
    if (!driver.parseArguments(argc, argv))
        return 0;

    script::Lexer lexer;
    script::Parser parser(lexer);
    
    script::Analysis analysis;
    std::unique_ptr<script::ASTProgram> program;
    try {
        lexer.setProgram(std::string(driver.filename));
        program = std::move(parser.parse());
        program->accept(&analysis);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    // Dump ast to file
    if (driver.dumpAST_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".ast";
        std::fstream dumpASTFile(dumpFilename, std::ofstream::out);
        script::DumpAST dumpAST(dumpASTFile);
        program->accept(&dumpAST);
    }
    
    // translate AST to IR (quad).
    script::IRModule module;
    script::Translator translator(module);
    translator.translate(program.get());

    if (driver.dumpQuad_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".quad";
        std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
        script::DumpQuad dumpQuad(dumpIRFile);
        dumpQuad.dump(module);
    }

    if (driver.dumpCFG_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".cfg";
        std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
        script::DumpCFG dumpCFG(dumpIRFile);
        dumpCFG.dump(module);
    }

    script::OpcodeContext opcode;
    script::CodeGenerator codegen(opcode);
    codegen.gen(module);
    
    driver.dumpOpcode_ = true;

    if (driver.dumpOpcode_)
    {
        std::string dumpFilename(driver.filename);
        dumpFilename += ".txt";
        std::fstream dumpOpcodeFile(dumpFilename, std::ofstream::out);
        script::DumpOpcode dumpByte(dumpOpcodeFile);
        dumpByte.dump(opcode.getOpcodes(), opcode.opcodeLength());
    }

	return 0;
}