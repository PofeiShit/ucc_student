#include <gtest/gtest.h>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>
#include "lex.h"
#include "input.h"
#include "alloc.h"
#include "str.h"
#include "input.h"
#include "vector.h"
#include "type.h"
#include "symbol.h"
#include "target.h"
#include "reg.h"
#include "ast.h"
#include "decl.h"

Heap CurrentHeap;
HEAP(StringHeap);
HEAP(ProgramHeap);
HEAP(FileHeap);
FILE *ASMFile;
char *ExtName = ".s";
char *ASMFileName = NULL;
namespace {

    void Initialize(void)
    {
        CurrentHeap = &FileHeap;
        InitSymbolTable();
        ASMFile = NULL;
    }
    void Finalize(void)
    {
        FreeHeap(&FileHeap);
    }
    unsigned char code[] = "static char int void struct "
        "return id_a 123 , = |= ^= &= <<= >>= += -= *= /= %= ( ) \"test\" { } ;";
    const int tokens[] = {TK_STATIC, TK_CHAR, TK_INT, TK_VOID, TK_STRUCT, 
        TK_RETURN, TK_ID, TK_INTCONST, TK_COMMA, TK_ASSIGN, 
        TK_BITOR_ASSIGN, TK_BITXOR_ASSIGN, TK_BITAND_ASSIGN, TK_LSHIFT_ASSIGN, TK_RSHIFT_ASSIGN, 
        TK_ADD_ASSIGN, TK_SUB_ASSIGN, TK_MUL_ASSIGN, TK_DIV_ASSIGN, TK_MOD_ASSIGN, 
        TK_LPAREN, TK_RPAREN, TK_STRING, TK_LBRACE, TK_RBRACE, 
        TK_SEMICOLON, TK_END};

    std::string get_string(std::string res)
    {
    　　int r = res.find('\r\n');
    　　while (r != std::string::npos)
    　　{
    　　　　if (r != std::string::npos)
    　　　　{
    　　　　　　res.replace(r, 1, "");
    　　　　　　r = res.find('\r\n');
    　　　　}
    　　}
    　　res.erase(std::remove_if(res.begin(), res.end(), ::isspace), res.end());
    　　return res;
    }
}
class TestUcl : public::testing::Test 
{
    virtual void SetUp()
    {
        CurrentHeap = &ProgramHeap;
    	SetupRegisters();
        SetupLexer();
        SetupTypeSystem();
    }
    virtual void TearDown()
    {
        ;
    }
};
TEST_F(TestUcl, Test_All_Token)
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

TEST_F(TestUcl, Test_Global_Declaration_Int)
{
    char *input = "./ucl/test_case/test_file/global_declaration_int.c";
    char output[] = "./ucl/test_case/test_file/global_declaration_int.s";
    ASMFileName = output;

    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                        ".data"
                        ".comm	a,4"
                        ".text\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    asmFile.close();
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
}
TEST_F(TestUcl, Test_Type)
{
    char *input = "./ucl/test_case/test_file/test_type.c";
    char *output = "./ucl/test_case/test_file/test_type.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                        ".data"
                        ".comm	ch,1"
                        ".text\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Comma_Expression)
{
    char *input = "./ucl/test_case/test_file/test_comma_expression.c";
    char *output = "./ucl/test_case/test_file/test_comma_expression.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                        ".data"
                        ".comm	a,4"
                        ".comm	b,4"
                        ".text\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Static)
{
    char *input = "./ucl/test_case/test_file/test_static.c";
    char *output = "./ucl/test_case/test_file/test_static.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                        ".data"
                        ".lcomm	a,4"
                        ".text\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}
TEST_F(TestUcl, Test_Function_Definition)
{
    char *input = "./ucl/test_case/test_file/test_function_definition.c";
    char *output = "./ucl/test_case/test_file/test_function_definition.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                            ".BB0:"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Function_Definition_With_Parameter)
{
    char *input = "./ucl/test_case/test_file/test_function_definition_with_parameter.c";
    char *output = "./ucl/test_case/test_file/test_function_definition_with_parameter.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                            ".BB0:"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Return)
{
    char *input = "./ucl/test_case/test_file/test_return.c";
    char *output = "./ucl/test_case/test_file/test_return.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                            ".BB0:"
                                "movl $0, %eax"
                                "jmp .BB2"
                            ".BB1:"
                            ".BB2:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_Function_Call)
{
    char *input = "./ucl/test_case/test_file/test_function_call.c";
    char *output = "./ucl/test_case/test_file/test_function_call.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	print_hw"
                            "print_hw:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                            ".BB0:"
                                "movl $0, %eax"
                                "jmp .BB2"
                            ".BB1:"
                            ".BB2:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $4, %esp"
                            ".BB3:"
                                "pushl $97"
                                "pushl $3"
                                "call print_hw"
                                "addl $8, %esp"
                                "movl $1, %eax"
                                "jmp .BB5"
                            ".BB4:"
                            ".BB5:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Local_Declaration)
{
    char *input = "./ucl/test_case/test_file/local_declaration.c";
    char *output = "./ucl/test_case/test_file/local_declaration.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $4, %esp"
                            ".BB0:"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_Struct)
{
    char *input = "./ucl/test_case/test_file/test_struct.c";
    char *output = "./ucl/test_case/test_file/test_struct.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".comm t,8"
                            ".text\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}

TEST_F(TestUcl, Test_Assign)
{
    char *input = "./ucl/test_case/test_file/test_assign.c";
    char *output = "./ucl/test_case/test_file/test_assign.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $4, %esp"
                            ".BB0:"
                                "movl $1, -4(%ebp)"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_BitOr_Assign)
{
    char *input = "./ucl/test_case/test_file/test_bitor_assign.c";
    char *output = "./ucl/test_case/test_file/test_bitor_assign.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $8, %esp"
                            ".BB0:"
                                "movl -4(%ebp), %eax"
                                "orl $1, %eax"
                                "movl %eax, -4(%ebp)"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_Sub_Assign)
{
    char *input = "./ucl/test_case/test_file/test_sub_assign.c";
    char *output = "./ucl/test_case/test_file/test_sub_assign.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $8, %esp"
                            ".BB0:"
                                "movl -4(%ebp), %eax"
                                "addl $-3, %eax"
                                "movl %eax, -4(%ebp)"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_Div_Assign)
{
    char *input = "./ucl/test_case/test_file/test_div_assign.c";
    char *output = "./ucl/test_case/test_file/test_div_assign.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $8, %esp"
                            ".BB0:"
                                "movl -4(%ebp), %eax"
                                "movl $3, %ecx"
                                "cdq"
                                "idivl %ecx"
                                "movl %eax, -4(%ebp)"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}


TEST_F(TestUcl, Test_Binary)
{
    char *input = "./ucl/test_case/test_file/test_binary.c";
    char *output = "./ucl/test_case/test_file/test_binary.s";
    ASMFileName = output;
    
    AstTranslationUnit transUnit;
    Initialize();

	transUnit = ParseTranslationUnit(input);
	CheckTranslationUnit(transUnit);
	Translate(transUnit);

    EmitTranslationUnit(transUnit);
    Finalize();

    std::string asmcode = "# Code auto-generated by UCC"
                            ".data"
                            ".text"
                            ".globl	main"
                            "main:"
                                "pushl %ebp"
                                "pushl %ebx"
                                "pushl %esi"
                                "pushl %edi"
                                "movl %esp, %ebp"
                                "subl $32, %esp"
                            ".BB0:"
                                "movl -4(%ebp), %eax"
                                "orl $3, %eax"
                                "movl -4(%ebp), %ecx"
                                "shll $3, %ecx"
                                "movl -4(%ebp), %edx"
                                "addl $3, %edx"
                                "movl -4(%ebp), %ebx"
                                "addl $-3, %ebx"
                                "movl -4(%ebp), %esi"
                                "imull $3, %esi"
                                "movl -4(%ebp), %eax"
                                "movl $3, %edi"
                                "cdq"
                                "idivl %edi"
                                "movl -4(%ebp), %eax"
                                "movl $3, %edi"
                                "cdq"
                                "idivl %edi"
                            ".BB1:"
                                "movl %ebp, %esp"
                                "popl %edi"
                                "popl %esi"
                                "popl %ebx"
                                "popl %ebp"
                                "ret\xff";
    std::string testcode;

    std::ifstream asmFile(output, std::ifstream::in);
    if (!asmFile.is_open()) {
        std::cout << "open:" << output << "failed\n";
    }
    char ch = asmFile.get();
    testcode.push_back(ch);
    while (asmFile.good()) {
        ch = asmFile.get();
        testcode.push_back(ch);
    }
    EXPECT_EQ(get_string(asmcode), get_string(testcode));
    asmFile.close();
}
