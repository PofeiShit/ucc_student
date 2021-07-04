#include "ucl.h"
#include "gen.h"

#include "ast.h"
#include "expr.h"
#include "input.h"

BBlock CurrentBB;
void AppendInst(IRInst inst)
{
	//assert(CurrentBB != NULL);

	CurrentBB->insth.prev->next = inst;
	inst->prev = CurrentBB->insth.prev;
	inst->next = &CurrentBB->insth;
	CurrentBB->insth.prev = inst;

	CurrentBB->ninst++;
}
void GenerateAssign(Type ty, Symbol dst, int opcode, Symbol src1, Symbol src2)
{
	IRInst inst;

	//assert(dst->kind == SK_Temp);

	ALLOC(inst);
	inst->ty = ty;
	inst->opcode = opcode;
	inst->opds[0] = dst;
	inst->opds[1] = src1;
	inst->opds[2] = src2;
	AppendInst(inst);
}

Symbol TryAddValue(Type ty, int op, Symbol src1, Symbol src2)
{
	Symbol t;

	t = CreateTemp(ty);

	GenerateAssign(ty, t, op, src1, src2);
	return t;
}

Symbol AddressOf(Symbol p)
{
	//assert(p->kind != SK_Temp);
	// p->addressed = 1;
	return TryAddValue(T(POINTER), ADDR, p, NULL); 
}
/**
	(1)
		The returned bblock of CreateBBlock(...) is not named yet.
	(2)	The naming operation is done later: 
		bb->sym is created later via CreateLabel() called 
	in TranslateFunction(AstFunction func).
	(3)	Some basic blocks will be merged later in Optimize(),
	no name is needed for them.
 */
BBlock CreateBBlock(void)
{
	BBlock bb;

	CALLOC(bb);

	bb->insth.opcode = NOP;
	bb->insth.prev = bb->insth.next = &bb->insth;
	return bb;
}

/**
	(1)
	When generating UIL or assembly code, we just have 
	to generate code  from top to down 
	in a file. So a block-list is efficient for such task.
	(2)
	This function add @bb into basic block list.
 */
void StartBBlock(BBlock bb)
{
	//IRInst lasti;

	CurrentBB->next = bb;
	bb->prev = CurrentBB;
	//lasti = CurrentBB->insth.prev;
	/**
		BB1:
			s1;
			......
			sn;
		BB2:
			t1;
			..
			tm
		If the above sn is not Jmp/IJMP,
			we are sure that  the control could flow directly from sn to t1;
			that is , a cfg-edge exists from BB1 to BB2 here,
			Even sn be a conditional jump.
		while sn is a Jmp/Ijmp, its target may be BB2, 
			we draw CfgEdge for them in GenerateJump(BBlock dstBB) and 
			GenerateIndirectJump(BBlock *dstBBs, int len, Symbol index)
			respectively.

		Warning:
			It seems that DrawCFGEdge don't check whether there is already 
			an same edge in the CFG.
	 */
	// if (lasti->opcode != JMP && lasti->opcode != IJMP)
	// {
	// DrawCFGEdge(CurrentBB, bb);
	// }
	CurrentBB = bb;
}

/**
	int f(int a, int b){
		
		return a+b;
	}
	
	int main(){
		int a = 3, b;
		b = f(a,5);
		return 0;
	}

	t0 = f(a, 5);
	b = t0;	
 */
void GenerateFunctionCall(Type ty, Symbol recv, Symbol faddr, Vector args)
{
	ILArg p;
	IRInst inst;

	ALLOC(inst);
	inst->ty = ty;
	inst->opcode = CALL;
	inst->opds[0] = recv;
	inst->opds[1] = faddr;
	inst->opds[2] = (Symbol)args;
	AppendInst(inst);

}
void GenerateReturn(Type ty, Symbol src)
{
	IRInst inst;
	ALLOC(inst);
	inst->ty = ty;
	inst->opcode = RET;
	inst->opds[0] = src;
	inst->opds[1] = inst->opds[2] = NULL;
	AppendInst(inst);
}
void GenerateJump(BBlock dstBB)
{
	IRInst inst;
	ALLOC(inst);

	inst->opcode = JMP;
	inst->opds[0] = (Symbol)dstBB;
	inst->opds[1] = inst->opds[2] = NULL;
	AppendInst(inst);
}
