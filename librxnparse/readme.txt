bison -p RP RP.y -o RP_yacc.cpp
flex -P RP --nounistd -o RP_lex.cpp RP.l


g++ -o test -DYYDEBUG=1 -DSTAND_ALONE RP_AstNode.cpp RP_lex.cpp RP_yacc.cpp RP_demo.cpp

