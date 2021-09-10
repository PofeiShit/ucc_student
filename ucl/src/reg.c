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
