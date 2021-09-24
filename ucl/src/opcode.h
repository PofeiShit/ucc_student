#ifndef OPCODE
#error "You must define OPCODE macro before include this file"
#endif
/**
	opcode here is used by UIL
 */
OPCODE(BOR,     "|",                    Assign)
OPCODE(BXOR,    "^",                    Assign)
OPCODE(BAND,    "&",                    Assign) 
OPCODE(LSH,     "<<",                   Assign)
OPCODE(RSH,     ">>",                   Assign)
OPCODE(ADD,     "+",                    Assign)
OPCODE(SUB,     "-",                    Assign)
OPCODE(MUL,     "*",                    Assign)
OPCODE(DIV,     "/",                    Assign)
OPCODE(MOD,     "%",                    Assign)
OPCODE(NEG,		"-",					Assign)
OPCODE(BCOM,	"~",					Assign)
OPCODE(JZ,      "",                     Branch)
OPCODE(JNZ,     "!",                    Branch)
OPCODE(JE,      "==",                   Branch)
OPCODE(JNE,     "!=",                   Branch)
OPCODE(JG,      ">",                    Branch)
OPCODE(JL,      "<",                    Branch)
OPCODE(JGE,     ">=",                   Branch)
OPCODE(JLE,     "<=",                   Branch)
OPCODE(MOV,     "=",                    Move)
OPCODE(RET, 	"ret", 					Return)
OPCODE(JMP, 	"jmp",					Jump)
OPCODE(CALL,    "call",                 Call)
OPCODE(ADDR,    "&",                    Address)
OPCODE(NOP, 	"NOP",					NOP)
