#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "gen.h"
#include "reg.h"
#include "target.h"
#include "output.h"

#define DST  inst->opds[0]
#define SRC1 inst->opds[1]
#define SRC2 inst->opds[2]

#define PRESERVE_REGS 4
#define SCRATCH_REGS  4
#define STACK_ALIGN_SIZE 4

enum ASMCode 
{
#define TEMPLATE(code, str) code,
#include "x86linux.tpl"
#undef TEMPLATE
};
static void EmitNOP(IRInst inst)
{
	;
}
static int GetListen(Symbol reg) 
{
	int len = 0;
	Symbol next = reg->link;
	while (next) {
		len++;
		next = next->link;
	}
	return len;
}

static void AddVarToReg(Symbol reg, Symbol v)
{
	// temp variable insert reg
	v->link = reg->link;
	reg->link = v;
	// variable use reg 
	v->reg = reg;
}
/**
 * Allocate register for instruction operand. index indicates which operand
 */
static void AllocateReg(IRInst inst, int index)
{
	Symbol reg;
	Symbol p;

	p = inst->opds[index];

	// In x86, UCC only allocates register for temporary
	if (p->kind != SK_Temp)
		return;

	// if it is already in a register, mark the register as used by current uil
	if (p->reg != NULL)
	{
		//UsedRegs |= 1 << p->reg->val.i[0];
		return;
	}

	reg = X86Regs[0];

	AddVarToReg(reg, p);
}

static void EmitPrologue(int stksize)
{
	/**
	 * regardless of the actual register usage, always save the preserve registers.
	 * on x86 platform, they are ebp, ebx, esi and edi
	 */
	PutASMCode(X86_PROLOGUE, NULL);
	if (stksize != 0)
	{
		Symbol sym = IntConstant(stksize);
		// TEMPLATE(X86_EXPANDF,  "subl %0, %%esp")
		PutASMCode(X86_EXPANDF, &sym);
	}
}

static void EmitEpilogue(int stksize)
{
	PutASMCode(X86_EPILOGUE, NULL);
}


static void PushArgument(Symbol p)
{

	PutASMCode(X86_PUSH, &p);

}
/**
	DST:
			return value
	SRC1:
			function name
	SRC2:
			arguments_list
*/
static void EmitCall(IRInst inst)
{
	Vector args;
	ILArg arg;
	int i, stksize;

	args = (Vector)SRC2;
	stksize = 0;

	for (i = LEN(args) - 1; i >= 0; --i)
	{
		arg = GET_ITEM(args, i);
		PushArgument(arg->sym);
		//stksize += ALIGN(arg->ty->size, STACK_ALIGN_SIZE);
		stksize += 4;
	}
	/**
		 We don't have to call ClearRegs() in EmitCall(),
		 Because ESI/EDI/EBX are saved in EmitPrologue().
	 */
	SpillReg(X86Regs[EAX]);
	SpillReg(X86Regs[ECX]);
	SpillReg(X86Regs[EDX]);

	PutASMCode(X86_CALL, inst->opds);
	if(stksize != 0){
		Symbol p;
		p = IntConstant(stksize);
		PutASMCode(X86_REDUCEF, &p);
	}


	if(DST == NULL){
		/**
			We have set X87Top to NULL in EmitReturn()
		 */
		return;
	}
}
/**
 * When a variable is modified, if it is not in a register, do nothing;
 * otherwise, spill othere variables in this register, set the variable's
 * needWB flag.(need write back to memory)
 */
#if 0
static void ModifyVar(Symbol p)
{
	Symbol reg;

	if (p->reg == NULL)
		return;

	reg = p->reg;
	/**
		The following assertion seems to be right.
		In fact, the only thing we have to do here 
		is set
		p->needwb = 1; 	?
	 */
	// assert(GetListLen(reg) == 1);
	// assert(p == reg->link);

	SpillReg(reg);
	
	AddVarToReg(reg, p);

}
#endif
static void EmitAddress(IRInst inst)
{
	//assert(DST->kind == SK_Temp && SRC1->kind != SK_Temp);
	AllocateReg(inst, 0);
	PutASMCode(X86_ADDR, inst->opds);
	//ModifyVar(DST);
}
static void (* Emitter[])(IRInst inst) = 
{
#define OPCODE(code, name, func) Emit##func, 
#include "opcode.h"
#undef OPCODE
};
static void EmitIRInst(IRInst inst)
{
	struct irinst instc = *inst;
	(* Emitter[inst->opcode])(&instc);
	return;
}
static void EmitBBlock(BBlock bb)
{
	IRInst inst = bb->insth.next;
	while (inst != &bb->insth)
	{
		//UsedRegs = 0;
		//  the kernel part of emit ASM from IR.
		EmitIRInst(inst);

		inst = inst->next;
	}
	ClearRegs();
}
void EmitFunction(FunctionSymbol fsym)
{
	BBlock bb;
	int stksize;
	FSYM = fsym;
	Export((Symbol)fsym);	
	DefineLabel((Symbol)fsym);
	stksize = 4;
	/**
		main:
		pushl %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		movl %esp, %ebp
		subl $32, %esp
	 */
	EmitPrologue(stksize);

	bb = fsym->entryBB;
	while (bb != NULL)
	{	
		// to show all basic blocks
		DefineLabel(bb->sym);
		EmitBBlock(bb);
		bb = bb->next;
	}
	/**
		movl %ebp, %esp
		popl %edi
		popl %esi
		popl %ebx
		popl %ebp
		ret
	 */
	EmitEpilogue(stksize);
	PutString("\n");
}
