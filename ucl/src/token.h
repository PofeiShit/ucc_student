#ifndef TOKEN
#error "You must define TOKEN macro before include this file"
#endif
TOKEN(TK_AUTO,		"auto")
TOKEN(TK_EXTERN, 	"extern")
TOKEN(TK_STATIC, 	"static")
TOKEN(TK_CHAR, 		"char")
TOKEN(TK_SHORT,		"short")

TOKEN(TK_INT, 		"int")
TOKEN(TK_VOID,      "void")
TOKEN(TK_STRUCT,    "struct")
TOKEN(TK_RETURN, 	"return")
//identifier
TOKEN(TK_ID,        "ID")

//constant
TOKEN(TK_INTCONST, "int")
//operators
TOKEN(TK_COMMA,			",")
TOKEN(TK_QUESTION,      "?")
TOKEN(TK_COLON,         ":")
TOKEN(TK_ASSIGN,        "=")
TOKEN(TK_BITOR_ASSIGN,  "|=")
TOKEN(TK_BITXOR_ASSIGN, "^=")

TOKEN(TK_BITAND_ASSIGN, "&=")
TOKEN(TK_LSHIFT_ASSIGN, "<<=")
TOKEN(TK_RSHIFT_ASSIGN, ">>=")
TOKEN(TK_ADD_ASSIGN,    "+=")
TOKEN(TK_SUB_ASSIGN,    "-=")

TOKEN(TK_MUL_ASSIGN,    "*=")
TOKEN(TK_DIV_ASSIGN,    "/=")
TOKEN(TK_MOD_ASSIGN,    "%=")
TOKEN(TK_OR,            "||")
TOKEN(TK_AND,           "&&")
TOKEN(TK_BITOR,         "|")
TOKEN(TK_BITXOR,        "^")
TOKEN(TK_BITAND,        "&")
TOKEN(TK_EQUAL,         "==")
TOKEN(TK_UNEQUAL,       "!=")
TOKEN(TK_GREAT,         ">")
TOKEN(TK_LESS,          "<")
TOKEN(TK_GREAT_EQ,      ">=")
TOKEN(TK_LESS_EQ,       "<=")
TOKEN(TK_LSHIFT,        "<<")
TOKEN(TK_RSHIFT,        ">>")
TOKEN(TK_ADD,           "+")
TOKEN(TK_SUB,           "-")
TOKEN(TK_MUL,           "*")
TOKEN(TK_DIV,           "/")
TOKEN(TK_MOD,           "%")
TOKEN(TK_LPAREN,        "(")

TOKEN(TK_RPAREN,        ")")
TOKEN(TK_STRING, 	"STR")
//punctuators
TOKEN(TK_LBRACE,        "{")
TOKEN(TK_RBRACE,        "}")
TOKEN(TK_SEMICOLON,     ";")

TOKEN(TK_ELLIPSIS, 		"...")
TOKEN(TK_NEWLINE,       "\n")
TOKEN(TK_END,           "EOF")
