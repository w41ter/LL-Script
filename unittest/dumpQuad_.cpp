#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "..\script\Parser\lexer.h"
#include "..\script\Parser\Parser.h"
#include "..\script\Semantic\Analysis.h"
#include "..\script\Semantic\dumpAST.h"
#include "..\script\Semantic\Translator.h"
#include "..\script\IR\QuadGenerator.h"
#include "..\script\IR\Quad.h"
#include "..\script\IR\dumpQuad.h"

int main(int argc, char* argv[])
{
    std::string name = "test.txt";
    if (argc == 2)
        name = argv[1];
    script::Lexer lexer;
    script::Parser parser(lexer);
    lexer.setProgram(name);
    
    script::Analysis analysis;
    std::unique_ptr<script::ASTProgram> program;
    try {
        program = std::move(parser.parse());
        std::cout << " begin analysis ast..." << std::endl;
        program->accept(&analysis);
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    std::cout << " Begin translater AST to IR..." << std::endl;
    // translate AST to IR (quad).
    script::QuadModule module;
    script::Translator translator(module);
    program->accept(&translator);

    std::cout << " Begin dump IR..." << std::endl;
    std::string dumpFilename(name);
    dumpFilename += ".ir";
    std::fstream dumpIRFile(dumpFilename, std::ofstream::out);
    script::DumpQuad dumpQuad(dumpIRFile);
    dumpQuad.dump(module);
    
    return 0;
}