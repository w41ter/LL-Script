#include <string>

#include "../script/Parser/Lexer.h"
#include "../script/Parser/Parser.h"

using std::string;
using namespace script;

int main() {
    string file("test.txt");
    Lexer lexer;
    lexer.setProgram(file);
    Parser parser(lexer);
    parser.parse();
    return 0;
}