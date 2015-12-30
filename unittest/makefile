cc = g++
other = -std=c++11 -c
test_lexer_object = lexer.o
test_parser_object = $(test_lexer_object) parser.o

testParser:  testParser.o $(test_parser_object)
    $(cc) -o testParser.exe $(test_parser_object)  testParser.o
    
testLexer : testLexer.o $(test_lexer_object) 
    $(cc) -o testLexer.exe $(test_lexer_object)  testLexer.o
    
testParser.o : testParser.cpp 
    $(cc) $(other) testParser.cpp
    
parser.o : ../script/Parser/parser.h ../script/Parser/parser.cpp
    $(cc) $(other) ../script/Parser/parser.cpp

testLexer.o : testLexer.cpp UnitTest.h
    $(cc) $(other) testLexer.cpp

lexer.o : ../script/Parser/lexer.cpp ../script/Parser/lexer.h
    $(cc) $(other) ../script/Parser/lexer.cpp
    
clean :
    rm *.o
    rm *.exe
    