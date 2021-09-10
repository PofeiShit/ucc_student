#include <gtest/gtest.h>
#include "lex.h"
#include "input.h"
#include "alloc.h"
#include "str.h"
#include "input.h"
Heap CurrentHeap;
HEAP(StringHeap);
HEAP(ProgramHeap);

namespace
{
    unsigned char code[] = "static char int void struct "
        "return id_a 123 , = |= ^= &= <<= >>= += -= *= /= %= ( ) \"test\" { } ;";
   const int tokens[] = {TK_STATIC, TK_CHAR, TK_INT, TK_VOID, TK_STRUCT, 
        TK_RETURN, TK_ID, TK_INTCONST, TK_COMMA, TK_ASSIGN, 
        TK_BITOR_ASSIGN, TK_BITXOR_ASSIGN, TK_BITAND_ASSIGN, TK_LSHIFT_ASSIGN, TK_RSHIFT_ASSIGN, 
        TK_ADD_ASSIGN, TK_SUB_ASSIGN, TK_MUL_ASSIGN, TK_DIV_ASSIGN, TK_MOD_ASSIGN, 
        TK_LPAREN, TK_RPAREN, TK_STRING, TK_LBRACE, TK_RBRACE, 
        TK_SEMICOLON, TK_END};
}
class TestLex : public::testing::Test 
{
    virtual void SetUp()
    {
        SetupLexer();
        CurrentHeap = &ProgramHeap;
    }
    virtual void TearDown()
    {
        ;
    }
};
TEST_F(TestLex, TestAllToken)
{
    Input.base = const_cast<unsigned char*>(code);
    Input.size = sizeof(code) / sizeof(code[0]);
    int sz = Input.size;
	Input.base[sz - 1] = END_OF_FILE;
	Input.cursor = Input.base;
    int tok = GetNextToken();
    int i = 0;
    while (tok != TK_END) {
        EXPECT_EQ(tok, tokens[i]);
        i++;
        tok = GetNextToken();
    }
}
