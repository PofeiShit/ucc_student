#ifndef __GEN_H_
#define __GEN_H_

enum OPCode
{
#define OPCODE(code, name, func) code, 
#include "opcode.h" 
#undef OPCODE
};
typedef struct irinst
{
	struct irinst *prev;
	struct irinst *next;
	Type ty;
	int opcode;
	Symbol opds[3];
} *IRInst;

// control flow graph edge
typedef struct cfgedge
{
	BBlock bb;
	struct cfgedge *next;
} *CFGEdge;

struct bblock
{
	struct bblock *prev;
	struct bblock *next;
	Symbol sym;
	struct irinst insth;
	int ninst;
	int ref;
};

typedef struct ilarg
{
	Symbol sym;
	Type ty;
} *ILArg;

void DefineTemp(Symbol t, int op, Symbol src1, Symbol src2);
Symbol AddressOf(Symbol sym);
Symbol Deref(Type ty, Symbol addr);
Symbol Simplify(Type ty, int op, Symbol src1, Symbol src2);
Symbol TryAddValue(Type ty, int op, Symbol src1, Symbol src2);
	
void GenerateMove(Type ty, Symbol dst, Symbol src);
void GenerateAssign(Type ty, Symbol dst, int opcode, Symbol src1, Symbol src2);
void GenerateIndirectMove(Type ty, Symbol dst, Symbol src);
void GenerateJump(BBlock dstBB);
void GenerateReturn(Type ty, Symbol src);
void GenerateFunctionCall(Type ty, Symbol recv, Symbol faddr, Vector args);
void GenerateBranch(Type ty, BBlock dstBB, int opcode, Symbol src1, Symbol src2);
void GenerateIndirectJump(BBlock *dstBBs, int len, Symbol index);
void GenerateClear(Symbol dst, int size);
BBlock CreateBBlock(void);
void   StartBBlock(BBlock bb);
extern BBlock CurrentBB;
extern int OPMap[];
#endif
