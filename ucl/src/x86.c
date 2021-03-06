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
extern int SwitchTableNum;
enum ASMCode 
{
#define TEMPLATE(code, str) code,
#include "x86linux.tpl"
#undef TEMPLATE
};

#define ASM_CODE(opcode, tcode) ((opcode << 2) + tcode - I4)
#define IsNormalRecord(rty) (rty->size != 1 && rty->size != 2 && rty->size != 4 && rty->size != 8)
static void Move(int code, Symbol dst, Symbol src)
{
	Symbol opds[2];
	opds[0] = dst;
	opds[1] = src;	
	PutASMCode(code, opds);
}
static Symbol PutInReg(Symbol p)
{
	Symbol reg;
	if (p->reg != NULL) {
		return p->reg;
	}
	reg = GetReg();
	Move(X86_MOVI4, reg, p);
	return reg;
}
static void EmitNOP(IRInst inst)
{
	;
}
static void EmitBranch(IRInst inst)
{
	int tcode = TypeCode(inst->ty);
	BBlock p = (BBlock)DST;

	DST = p->sym;
	SRC1->ref--; // we should minus SK_Temp ref, not register
	// SRC2存在且为常数，可以不必把SRC1的值加载到寄存器中。否则把SRC1加载到寄存器中
	if (SRC2) {
		if (SRC2->kind != SK_Constant) 
			SRC1 = PutInReg(SRC1); // here SRC1 may be a register
	}
	// SRC1已经在寄存器中，中间指令的源操作数SRC1改为SRC1->reg。
	if (SRC1->reg != NULL) {
		SRC1 = SRC1->reg;
	}
	// SRC2同上
	if (SRC2)
	{
		SRC2->ref--;
		if (SRC2->reg != NULL)
			SRC2 = SRC2->reg;
	}
	ClearRegs();
	PutASMCode(ASM_CODE(inst->opcode, tcode), inst->opds);

}
static void EmitJump(IRInst inst)
{
	BBlock p = (BBlock)DST;
	DST = p->sym;
	ClearRegs();
	PutASMCode(X86_JMP, inst->opds);
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
		UsedRegs |= 1 << p->reg->val.i[0];
		return;
	}
	// SRC1->ref == 1: SRC1 only used by DST
	// SRC1->reg != NULL: SRC1 is already in register
	if (index == 0 && SRC1->ref == 1 && SRC1->reg != NULL) {
		reg = SRC1->reg;
		reg->link = NULL;
		AddVarToReg(reg, p);
		return ;
	}
	reg = GetReg();
	if (index != 0) {
		Move(X86_MOVI4, reg, p);
	}
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
		// TEMPLATE(X86_EXPANDF,  "subl %0, %%esp")E
		PutASMCode(X86_EXPANDF, &sym);
	}
}

static void EmitEpilogue(int stksize)
{
	PutASMCode(X86_EPILOGUE, NULL);
}

static void PushArgument(Symbol p, Type ty)
{
	int tcode = TypeCode(ty);
	if (tcode == B)
	{
		Symbol opds[3];

		SpillReg(X86Regs[ESI]);
		SpillReg(X86Regs[EDI]);
		SpillReg(X86Regs[ECX]);
		opds[0] = p;
		opds[1] = IntConstant(ty->size);
		opds[2] = IntConstant(ALIGN(ty->size, STACK_ALIGN_SIZE));
		PutASMCode(X86_PUSHB, opds);
	} else {
		PutASMCode(X86_PUSH, &p);
	}
}
/**
 * When a variable is modified, if it is not in a register, do nothing;
 * otherwise, spill othere variables in this register, set the variable's
 * needWB flag.(need write back to memory)
 */
static void ModifyVar(Symbol p)
{
	Symbol reg;

	if (p->reg == NULL)
		return;
	p->needwb = 0;
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
	p->needwb = 1;
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
	Type rty;
	int i, stksize;

	args = (Vector)SRC2;
	stksize = 0;
	rty = inst->ty;

	for (i = LEN(args) - 1; i >= 0; --i)
	{
		arg = (ILArg)GET_ITEM(args, i);
		PushArgument(arg->sym, arg->ty);
		if (arg->sym->kind != SK_Function) 
			arg->sym->ref--;
		stksize += ALIGN(arg->ty->size, STACK_ALIGN_SIZE);
	}
	/**
		 We don't have to call ClearRegs() in EmitCall(),
		 Because ESI/EDI/EBX are saved in EmitPrologue().
	 */
	SpillReg(X86Regs[EAX]);
	SpillReg(X86Regs[ECX]);
	SpillReg(X86Regs[EDX]);
	// IsNormalRecord: size of record must > 8
	if (IsRecordType(rty) && IsNormalRecord(rty)) {
		Symbol opds[2];
		opds[0] = GetReg();
		opds[1] = DST;
		PutASMCode(X86_ADDR, opds); // DST = recv(返回值)，取返回值地址
		PutASMCode(X86_PUSH, opds); // 返回值地址入栈
		stksize += 4;
		DST = NULL;
	}
	PutASMCode(SRC1->kind == SK_Function ? X86_CALL : X86_ICALL, inst->opds);

	if(stksize != 0){
		Symbol p;
		p = IntConstant(stksize);
		PutASMCode(X86_REDUCEF, &p);
	}
	if (DST != NULL) DST->ref--;
	if (SRC1->kind != SK_Function) SRC1->ref--;
	if(DST == NULL){
		/**
			We have set X87Top to NULL in EmitReturn()
		 */
		return;
	}
	switch (rty->size) {
	case 1:
		Move(X86_MOVI1, DST, X86ByteRegs[EAX]);
		break;
	case 2:
		Move(X86_MOVI2, DST, X86WordRegs[EAX]);
		break;
	case 4:
		AllocateReg(inst, 0);
		if (DST->reg != X86Regs[EAX]) {
			Move(X86_MOVI4, DST, X86Regs[EAX]);
		}
		ModifyVar(DST);
		break;
	case 8:
		Move(X86_MOVI4, DST, X86Regs[EAX]);
        Move(X86_MOVI4, CreateOffset(T(INT), DST, 4), X86Regs[EDX]);
		break;
	default:
		assert(0);
	}
}

static void EmitAddress(IRInst inst)
{
	assert(DST->kind == SK_Temp && SRC1->kind != SK_Temp);
	AllocateReg(inst, 0);
	PutASMCode(X86_ADDR, inst->opds);
	ModifyVar(DST);
}
static void EmitMoveBBlock(IRInst inst)
{
	if (inst->ty->size == 0)
		return ;
	SpillReg(X86Regs[EDI]);
	SpillReg(X86Regs[ESI]);
	SpillReg(X86Regs[ECX]);
	
	SRC2 = IntConstant(inst->ty->size);
	PutASMCode(X86_MOVB, inst->opds);
}
/**
 * Emit assembly code for move
 */
static void EmitMove(IRInst inst)
{
	int tcode = TypeCode(inst->ty);
	Symbol reg;
	if (tcode == B) {
		EmitMoveBBlock(inst);
		return ;
	}
	switch (tcode)
	{
		case I1:
		case U1:
			if (SRC1->kind == SK_Constant)
				Move(X86_MOVI1, DST, SRC1);
			else {
				reg = GetByteReg();
				Move(X86_MOVI1, reg, SRC1);
				Move(X86_MOVI1, DST, reg);
			}
			break;
		case I2:
		case U2:
			if (SRC1->kind == SK_Constant)
				Move(X86_MOVI2, DST, SRC1);
			else {
				reg = GetWordReg();
				Move(X86_MOVI2, reg, SRC1);
				Move(X86_MOVI2, DST, reg);
			}
			break;
		case I4:
		case U4:
			if (SRC1->kind == SK_Constant)
				Move(X86_MOVI4, DST, SRC1);
			else {
				/**
					we try to reuse the temporary value in register.
				*/			
				AllocateReg(inst, 1);
				AllocateReg(inst, 0);
				if (SRC1->reg == NULL && DST->reg == NULL)
				{
					/**
						On X86, we can't move from mem1 to mem2.
						So we have to move from mem1 to register , and 
						then from register to mem2.
					**/
					reg = GetReg();
					Move(X86_MOVI4, reg, SRC1);
					Move(X86_MOVI4, DST, reg);
				}
				else
				{
					if (SRC1->reg != DST->reg)
						Move(X86_MOVI4, DST, SRC1);
				}
			}
         	ModifyVar(DST);
			break;
		default:
			assert(0);
	}
}
static void EmitAssign(IRInst inst)
{
	int code;
	int tcode = TypeCode(inst->ty);

	code = ASM_CODE(inst->opcode, tcode);
	switch(code)
	{
	case X86_DIVI4:
	case X86_MODI4:
	case X86_DIVU4:
	case X86_MODU4:
	case X86_MULU4:
		// *和/ SRC1必须分配加载到寄存器%eax中，所以如果SRC1已经分配%eax了，需要把原先%eax的值回写。如果没有则也需要回写，然后再把SRC1的值mov到%eax中
		if (SRC1->reg == X86Regs[EAX]) {
			SRC1->needwb = 0;
			SpillReg(X86Regs[EAX]);
		} else {
			SpillReg(X86Regs[EAX]);
			Move(X86_MOVI4, X86Regs[EAX], SRC1);
		}
		// edx符号扩展为，必须先把edx原先的内容回写
		SpillReg(X86Regs[EDX]);
		UsedRegs = 1 << EAX | 1 << EDX;
		if (SRC2->kind == SK_Constant) {
			// idivl $10 illegal
			Symbol reg = GetReg();
			Move(X86_MOVI4, reg, SRC2);
			SRC2 = reg;
		} else {
			// AllocateReg只给临时变量分配寄存器，常量就用GetReg，然后Move
			AllocateReg(inst, 2);
		}
		PutASMCode(code, inst->opds);
		// 余数放在%edx中，将%edx长期分配给dst，之后回写或者其他用到dst的时候，对应寄存器edx
		if (code == X86_MODI4 || code == X86_MODU4)
			AddVarToReg(X86Regs[EDX], DST);
		else
			AddVarToReg(X86Regs[EAX], DST);
		break;
	case X86_LSHI4:
	case X86_LSHU4:
	case X86_RSHI4:
	case X86_RSHU4:
		AllocateReg(inst, 1);
		if (SRC2->kind != SK_Constant) {
			if (SRC2->reg != X86Regs[ECX]) {
				SpillReg(X86Regs[ECX]);
				Move(X86_MOVI4, X86Regs[ECX], SRC2);
			}
			SRC2 = X86ByteRegs[ECX];
			UsedRegs = 1 << ECX;
		}
		goto put_code;

	case X86_NEGI4:
	case X86_COMPI4:
		AllocateReg(inst, 1);
		goto put_code;
	default:
		AllocateReg(inst, 1);
		AllocateReg(inst, 2);
put_code:
		AllocateReg(inst, 0);
		if (DST->reg != SRC1->reg)
		{
			Move(X86_MOVI4, DST, SRC1);
		}
		PutASMCode(code, inst->opds);
		break;
	}
	ModifyVar(DST);
}
static void EmitCast(IRInst inst)
{
	Symbol dst, reg;
	int code;
	dst = DST;
	reg = NULL;
	code = inst->opcode + X86_EXTI1 - EXTI1;
	switch(code) 
	{
	case X86_EXTI1:
	case X86_EXTU1:
	case X86_EXTI2:
	case X86_EXTU2:
		// I1, I2, U1, U2 => I4
		AllocateReg(inst, 0);
		if (DST->reg == NULL) {
			DST = GetReg();
		}
		PutASMCode(code, inst->opds);
		if (dst != DST) {
			Move(X86_MOVI4, dst, DST);
		}
		break;

	case X86_TRUI1:
		if (SRC1->reg != NULL) {
			reg = X86ByteRegs[SRC1->reg->val.i[0]];
		}
		if (reg == NULL) {
			reg = GetByteReg();
			Move(X86_MOVI4, X86Regs[reg->val.i[0]], SRC1);
		}
		Move(X86_MOVI1, DST, reg);
		break;
	case X86_TRUI2:
		if (SRC1->reg != NULL) {
			reg = X86WordRegs[SRC1->reg->val.i[0]];
		}
		if (reg == NULL) {
			reg = GetWordReg();
			Move(X86_MOVI4, X86Regs[reg->val.i[0]], SRC1);
		}
		Move(X86_MOVI2, DST, reg);
		break;		
	default:
		;
	}

}

static void EmitIndirectJump(IRInst inst)
{
	BBlock *p;
	Symbol swtch;
	int len;
	Symbol reg;
	p = (BBlock*)DST;
	reg = PutInReg(SRC1);
	PutString("\n");
	Segment(DATA);
	CALLOC(swtch);
	swtch->kind = SK_Variable;
	swtch->ty = T(POINTER);
	swtch->name = FormatName("swtchTable%d", SwitchTableNum++);
	swtch->sclass = TK_STATIC;
	swtch->level = 0;
	DefineGlobal(swtch);

	DST = swtch;
	len = strlen(DST->aname);
	while (*p != NULL) {
		DefineAddress((*p)->sym);
		PutString("\n");
		LeftAlign(ASMFile, len);
		PutString("\t");
		p++;
	}

	PutString("\n");
	Segment(CODE);
	SRC1 = reg;
	ClearRegs();
	PutASMCode(X86_IJMP, inst->opds);

}
static void EmitIndirectMove(IRInst inst)
{
	Symbol reg;
	reg = PutInReg(DST);
	inst->opcode = MOV;
	DST = reg->next;
	EmitMove(inst);
}

static void EmitReturn(IRInst inst)
{
	Type ty = inst->ty;
	if (IsRecordType(ty) && IsNormalRecord(ty)) {
		inst->opcode = IMOV;
		SRC1 = DST; // return dt2; SRC1=DST=dt2
		DST = FSYM->params; // real return address is in fsym params.
		EmitIndirectMove(inst);
		return ;
	}
	switch (ty->size) 
	{
	case 1:
		Move(X86_MOVI1, X86ByteRegs[EAX], DST);
		break;
	case 2: // "movw %1, %0"
    	Move(X86_MOVI2, X86WordRegs[EAX], DST);
    	break;
	case 4:
		if (DST->reg != X86Regs[EAX]) {
			Move(X86_MOVI4, X86Regs[EAX], DST);	
		}
		break;
	case 8:
		Move(X86_MOVI4, X86Regs[EAX], DST);
    	Move(X86_MOVI4, X86Regs[EDX], CreateOffset(T(INT), DST, 4));
    	break;
	default:
		assert(0);
	}
}

static void EmitDeref(IRInst inst)
{
	Symbol reg;
	reg = PutInReg(SRC1);

	inst->opcode = MOV;
	SRC1 = reg->next;
	EmitMove(inst);
	ModifyVar(DST);
	return;
}
static void EmitClear(IRInst inst)
{
	int size = SRC1->val.i[0];
	Symbol p = IntConstant(0);

	switch(size) {
	case 1:
		Move(X86_MOVI1, DST, p);
		break;
	case 2:
		Move(X86_MOVI2, DST, p);
		break;
	case 4:
		Move(X86_MOVI4, DST, p);
		break;

	default:
		SpillReg(X86Regs[EAX]);
		PutASMCode(X86_CLEAR, inst->opds);
		break;
	}
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
		// 为一条中间代码生成汇编指令前，都会把变量UsedRegs清零
		UsedRegs = 0;
		//  the kernel part of emit ASM from IR.
		EmitIRInst(inst);
		if ( !(inst->opcode >= JZ && inst->opcode <= IJMP) && (inst->opcode != CALL)) {
			DST->ref--;
			if (SRC1 && SRC1->kind != SK_Function) SRC1->ref--;
			if (SRC2 && SRC2->kind != SK_Function) SRC2->ref--;
		}
		inst = inst->next;
	}
	ClearRegs();
}

static int LayoutFrame(FunctionSymbol fsym, int fstParamPos)
{
	Symbol p;
	int offset;
	offset = fstParamPos * STACK_ALIGN_SIZE;
	p = fsym->params;
	while (p) 
	{
		AsVar(p)->offset = offset;
		if (p->ty->size == 0)
			offset += ALIGN(EMPTY_OBJECT_SIZE, STACK_ALIGN_SIZE);
		else
			offset += ALIGN(p->ty->size, STACK_ALIGN_SIZE);
		p = p->next;
	}
	offset = 0;	
	p = fsym->locals;
	while (p) 
	{
		if (p->ty->size == 0) 
			offset += ALIGN(EMPTY_OBJECT_SIZE, STACK_ALIGN_SIZE);
		else
			offset += ALIGN(p->ty->size, STACK_ALIGN_SIZE);
		AsVar(p)->offset = -offset;
		// printf(" offset = %d, name = %s\n",AsVar(p)->offset,AsVar(p)->name);
		p = p->next;
	}
	return offset;
}

void EmitFunction(FunctionSymbol fsym)
{
	BBlock bb;
	Type rty;
	int stksize;

	FSYM = fsym;
	if (fsym->sclass != TK_STATIC)
		Export((Symbol)fsym);

	DefineLabel((Symbol)fsym);

	rty = fsym->ty->bty;
	if (IsRecordType(rty) && IsNormalRecord(rty)) {
		VariableSymbol p;
		CALLOC(p);
		p->kind = SK_Variable;
		p->name = "recvaddr";
		p->ty = T(POINTER);
		p->level = 1;
		p->sclass = TK_AUTO;

		p->next = fsym->params;
		fsym->params = (Symbol)p;
	}
	stksize = LayoutFrame(fsym, PRESERVE_REGS + 1);
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

void StoreVar(Symbol reg, Symbol v)
{
	Move(X86_MOVI4, v, reg);
}
