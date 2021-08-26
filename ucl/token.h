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
TOKEN(TK_ASSIGN,        "=")
TOKEN(TK_EQUAL,         "==")
TOKEN(TK_UNEQUAL,       "!=")

TOKEN(TK_GREAT,         ">")
TOKEN(TK_LESS,          "<")
TOKEN(TK_GREAT_EQ,      ">=")
TOKEN(TK_LESS_EQ,       "<=")
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
