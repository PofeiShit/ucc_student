#ifndef OPCODE
#error "You must define OPCODE macro before include this file"
#endif
/**
	opcode here is used by UIL
 */
 OPCODE(MOV,     "=",                    Move)
 OPCODE(RET, 	"ret", 					Return)
 OPCODE(JMP, 	"jmp",					Jump)
 OPCODE(CALL,    "call",                 Call)
 OPCODE(ADDR,    "&",                    Address)
 OPCODE(NOP, 	"NOP",					NOP)
