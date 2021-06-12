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
	//Type ty;
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
};

typedef struct ilarg
{
	Symbol sym;
	//Type ty;
} *ILArg;

BBlock CreateBBlock(void);
void   StartBBlock(BBlock bb);
extern BBlock CurrentBB;
#endif
