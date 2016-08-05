cc = g++
other = -std=c++14 -c
test_lexer_object = lexer.o
test_parser_object = $(test_lexer_object) parser.o
front_object = $(test_parser_object) analysis.o
ir_object = $(front_object) quadgenerator.o quad.o translator.o

dumpQuad : dumpQuad_.o dumpQuad.o $(ir_object)
  $(cc) -o dumpQuad.exe $(ir_object) dumpQuad_.o dumpQuad.o 

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
    
dumpQuad_.o : dumpQuad_.cpp
    $(cc) $(other) dumpQuad_.cpp 
    
quad.o : ../script/IR/Quad.h ../script/IR/Quad.cpp
    $(cc) $(other) ../script/IR/Quad.cpp
    
quadgenerator.o : ../script/IR/quadgenerator.h ../script/IR/quadgenerator.cpp
    $(cc) $(other) ../script/IR/quadgenerator.cpp
    
dumpQuad.o : ../script/IR/dumpQuad.h ../script/IR/dumpQuad.cpp
    $(cc) $(other) ../script/IR/dumpQuad.cpp
    
analysis.o : ../script/Semantic/AST.h ../script/Semantic/Analysis.h \
             ../script/Semantic/Analysis.cpp
    $(cc) $(other) ../script/Semantic/Analysis.cpp

translator.o : ../script/Semantic/translator.h ../script/Semantic/translator.cpp
    $(cc) $(other) ../script/Semantic/translator.cpp
    
clean :
    rm *.o
    rm *.exe
    