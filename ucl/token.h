#ifndef TOKEN
#error "You must define TOKEN macro before include this file"
#endif
TOKEN(TK_STATIC, 	"static")
TOKEN(TK_VOID,      "void")
//identifier
TOKEN(TK_ID,        "ID")

//operators
TOKEN(TK_LPAREN,        "(")
TOKEN(TK_RPAREN,        ")")

TOKEN(TK_STRING, 	"STR")

//punctuators
TOKEN(TK_LBRACE,        "{")
TOKEN(TK_RBRACE,        "}")
TOKEN(TK_SEMICOLON,     ";")

TOKEN(TK_NEWLINE,       "\n")
TOKEN(TK_END,           "EOF")
