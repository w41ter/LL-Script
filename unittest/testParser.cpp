#include <string>
#include <iostream>

#include "../script/Parser/Lexer.h"
#include "../script/Parser/Parser.h"

using std::string;
using namespace script;

int main() {
    string file("test.txt");
    Lexer lexer;
    lexer.setProgram(file);
    Parser parser(lexer);
    try {
        parser.parse();
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
    
    return 0;
}