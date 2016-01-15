cc = g++
other = -std=c++14 -c
test_lexer_object = lexer.o
test_parser_object = $(test_lexer_object) parser.o

dumpAST: dumpAST_.o $(test_parser_object) dumpAST.o
    $(cc) -o dumpAST.exe $(test_parser_object) dumpAST_.o dumpAST.o
  
testParser:  testParser.o $(test_parser_object)
    $(cc) -o testParser.exe $(test_parser_object)  testParser.o
    
testLexer : testLexer.o $(test_lexer_object) 
    $(cc) -o testLexer.exe $(test_lexer_object)  testLexer.o
    
testParser.o : testParser.cpp 
    $(cc) $(other) testParser.cpp
    
parser.o : ../script/Parser/parser.h ../script/Parser/parser.cpp ../script/Semantic/AST.h
    $(cc) $(other) ../script/Parser/parser.cpp

testLexer.o : testLexer.cpp UnitTest.h
    $(cc) $(other) testLexer.cpp

lexer.o : ../script/Parser/lexer.cpp ../script/Parser/lexer.h
    $(cc) $(other) ../script/Parser/lexer.cpp
    
dumpAST_.o : dumpAST_.cpp
    $(cc) $(other) dumpAST_.cpp
  
dumpAST.o : ../script/Semantic/dumpAST.cpp ../script/Semantic/dumpAST.h \
            ../script/Semantic/AST.h
    $(cc) $(other) ../script/Semantic/dumpAST.cpp
    

clean :
    rm *.o
    rm *.exe
    