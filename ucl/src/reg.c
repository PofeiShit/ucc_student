#include "ucl.h"
#include "gen.h"
#include "reg.h"
#include "target.h"
#include "output.h"
#include "ast.h"
#include "expr.h"
#include "input.h"

Symbol X86Regs[EDI + 1];
Symbol X86ByteRegs[EDI + 1];
/**
	#define	b(i)	(b[i]+b[i+1])
	 int main(){
		 int b[11];
		 int a = b(1) + (b(2) + (b(3) + (b(4) + (b(5) + (b(6)
							  + (b(7)+ (b(8)+ (b(9) + (b(0)+0)+b(1))+b(2))
									 +b(3))+b(4))+b(5))+b(6))+b(7))+b(8));
		 return 0;
	 }
*/
int UsedRegs;
void SpillReg(Symbol reg)
{
	reg->link = NULL;
}

/**
	When we are leaving a basic block,
	this function is calling to spill all the registers .
 */
void ClearRegs(void)
{
	int i;

	for (i = EAX; i <= EDI; ++i)
	{
		if (X86Regs[i])
			SpillReg(X86Regs[i]);
	}
}
Symbol CreateReg(char *name, char *iname, int no)
{
	Symbol reg;
	CALLOC(reg);

	reg->kind = SK_Register;
	reg->name = reg->aname = name;
	reg->val.i[0] = no;
	reg->reg = reg;
	
	if (iname != NULL) {
		CALLOC(reg->next);
		reg->next->kind = SK_IRegister;
		reg->next->name = iname;
	}
	return reg;
}
static int FindEmptyRegs(int endr)
{
	int i;
	for (i = EAX; i <= endr; i++) {
		//  esp and ebp 			寄存器中没有保存临时变量的值	该寄存器还没有分配给当前中间代码, <<优先级大于&
		if (X86Regs[i] != NULL && X86Regs[i]->link == NULL && !(1 << i & UsedRegs))
			return i;
	}
	return NO_REG;
}
static Symbol GetRegInternal(int width)
{
	int endr, i;
	Symbol *regs;
	switch(width) {
		case 4:
			endr = EDI;
			regs = X86Regs;
			break;
	}
	i = FindEmptyRegs(endr);
	UsedRegs |= 1 << i;

	return regs[i];
}
Symbol GetReg(void)
{
	return GetRegInternal(4);
}

