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
#include "Semantic\ASTContext.h"
#include "IR\QuadGenerator.h"
#include "IR\dumpQuad.h"
#include "IR\dumpCFG.h"
#include "IR\CodeGen.h"
#include "Runtime\opcode.h"
#include "Runtime\dumpOpcode.h"
#include "Runtime\VM.h"
#include "BuildIn\BuildIn.h"
#include "CompilerInstance.h"

int main(int argc, char* argv[])
{
    script::CompilerInstance compiler;
    auto &diag = compiler.getDiagnosisConsumer();
    diag.setMode(true);

    int length = 0;
    script::Byte *opcodes;
    {
        script::Driver &driver = compiler.getDriver();
        if (!driver.parseArguments(argc, argv))
            return 0;

        script::IRModule module;
        {
            // parser program
            script::ASTContext context;
            try {
                script::Lexer lexer(diag);
                script::Parser parser(lexer, context);
                lexer.setProgram(std::string(driver.filename));
                parser.parse();

                int error = diag.errors(), warning = diag.warnings();
                std::cout << "error(" << error << "), warning(" << warning << ")" << std::endl;
                if (error) return 0;

                script::Analysis analysis;
                analysis.analysis(context);
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
                dumpAST.dump(context);
            }

            // translate AST to IR (quad).
            script::Translator translator(module);
            translator.translate(context);
        }

        // dump quad to file
        if (driver.dumpQuad_)
        {
            std::string dumpFilename(driver.filename);
            dumpFilename += ".quad";
            std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
            script::DumpQuad dumpQuad(dumpIRFile);
            dumpQuad.dump(module);
        }

        // dump CFG to file
        if (driver.dumpCFG_)
        {
            std::string dumpFilename(driver.filename);
            dumpFilename += ".cfg";
            std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
            script::DumpCFG dumpCFG(dumpIRFile);
            dumpCFG.dump(module);
        }

        // generate opcode
        script::OpcodeContext opcode;
        script::CodeGenerator codegen(opcode);
        codegen.gen(module);
        opcodes = opcode.getOpcodes(length);

        // dump opcode to file
        if (driver.dumpOpcode_)
        {
            std::string dumpFilename(driver.filename);
            dumpFilename += ".txt";
            std::fstream dumpOpcodeFile(dumpFilename, std::ofstream::out);
            script::DumpOpcode dumpByte(dumpOpcodeFile);
            dumpByte.dump(opcodes, length);
        }
    }

    try {
        script::VirtualMachine vm;
        buildin::BuildIn::getInstance().bindGC(vm.getGC());
        vm.excute(opcodes, length);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    getchar();
	return 0;
}