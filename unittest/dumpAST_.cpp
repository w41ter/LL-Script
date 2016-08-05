#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "..\script\Parser\lexer.h"
#include "..\script\Parser\Parser.h"
#include "..\script\Semantic\dumpAST.h"

int main(int argc, char* argv[])
{
    std::string name = "test.txt";
    if (argc == 2)
        name = argv[1];
    script::Lexer lexer;
    script::Parser parser(lexer);
    lexer.setProgram(name);
    
    std::unique_ptr<script::ASTProgram> program;
    try {
        program = std::move(parser.parse());
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    std::string dumpFilename(name);
    dumpFilename += ".ast";
    std::fstream dumpASTFile(dumpFilename, std::ofstream::out);
    if (dumpASTFile)
    {
        std::cout << "Open file :" << dumpFilename << " to dump ast" << std::endl;
    }
    else
    {
        std::cout << "Open file is false :" << dumpFilename << std::endl;
        return 0;
    }
    script::DumpAST dumpAST(dumpASTFile);
    try {
        program->accept(&dumpAST);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
    
	return 0;
}