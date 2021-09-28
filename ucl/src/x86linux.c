#include "ucl.h"
#include "target.h"
#include "reg.h"
#include "output.h"

static char *ASMTemplate[] =
{
#define TEMPLATE(code, str) str, 
#include "x86linux.tpl"
#undef TEMPLATE
};

/**
	return a symbol @p 's name to be used in assembly code.
		4		---->	$4
		str0		---->	.str0
 */
static char* GetAccessName(Symbol p)
{
	if (p->aname != NULL)
		return p->aname;
	switch (p->kind)
	{
	case SK_Constant:
		p->aname = FormatName("$%s", p->name);
		break;
	case SK_String:
	case SK_Label:
		// .str0:	.string	"%d \012"		
		p->aname = FormatName(".%s", p->name);
		break;
	case SK_Variable:	
	case SK_Temp:
		if (p->level == 0 || p->sclass == TK_EXTERN){
			p->aname = p->name;
		}
		else if (p->sclass == TK_STATIC)
		{
			/**
				int main(){	
					static int c = 5;
					...
				}
				c.0:	.long	5	

				Because it is illegal to declare a global var in C language as following:
					int c.0;	
				So there is no conflict to generate a name 'c.0' for static variable in 
				assembly output.
			 */
			p->aname = FormatName("%s.%d", p->name, TempNum++);
		}
		else {
			p->aname = FormatName("%d(%%ebp)", AsVar(p)->offset);
		}
		break;
	case SK_Function:
		/**
			.globl	f
			f:
		**/
		p->aname = p->name;
		break;
	case SK_Offset:
		{
			Symbol base = p->link;
			int n = AsVar(p)->offset;
			n += AsVar(base)->offset;
			p->aname = FormatName("%d(%%ebp)", n);
		}
		break;

	default:
		;
	}
	return p->aname;
}

void DefineGlobal(Symbol p)
{
	Print("%s:\t", GetAccessName(p));
}
void Export(Symbol p)
{
	Print(".globl\t%s\n\n", GetAccessName(p));
}
/**
	 .str0:  .string "%d \012"
	 .str1:  .string "a + b + c + d = %d.\012"

 */
void DefineString(String str, int size)
{
	int i = 0;
	if(str->chs[size-1] == 0)
	{		
		PutString(".string\t\"");
		size--;
	} else {
		PutString(".ascii\t\"");
	}

	while (i < size)
	{
		// if it is not printable.	ASCII value  <= 0x20 ?
		if (! isprint(str->chs[i]))
		{
			/**
				printf("\\%03o", (unsigned char)12);
					\014o
			 */
			Print("\\%03o", (unsigned char)str->chs[i]);
		}
		else
		{
			if (str->chs[i] == '"')
			{
				
				//  \"
				PutString("\\\"");
			}
			else if (str->chs[i] == '\\')
			{
				/**
						When "make test",  Ugly Bug.
						//	\\
						PutString("\\\\");
 				 */
				PutString("\\\\");
			}
			else 
			{
				PutChar(str->chs[i]);
			}
		}
		i++;
	}
	PutString("\"\n");
}

void BeginProgram(void)
{
	int i;

	//ORG = 0;
	for (i = EAX; i <= EDI; ++i)
	{
		// Initialize register symbols to
		// make sure that no register contains data from variables.
		if (X86Regs[i] != NULL)
		{
			X86Regs[i]->link = NULL;
		}
	}

	PutString("# Code auto-generated by UCC\n\n");
}

void Segment(int seg)
{
	if (seg == DATA)
	{
		PutString(".data\n\n");
	}
	else if (seg == CODE)
	{
		PutString(".text\n\n");
	}
}

void SetupRegisters(void)
{
	X86Regs[EAX] = CreateReg("%eax", "(%eax)", EAX);
	X86Regs[EBX] = CreateReg("%ebx", "(%ebx)", EBX);
	X86Regs[ECX] = CreateReg("%ecx", "(%ecx)", ECX);
	X86Regs[EDX] = CreateReg("%edx", "(%edx)", EDX);
	X86Regs[ESI] = CreateReg("%esi", "(%esi)", ESI);
	X86Regs[EDI] = CreateReg("%edi", "(%edi)", EDI);

	X86ByteRegs[EAX] = CreateReg("%al", "NULL", EAX);
	X86ByteRegs[EBX] = CreateReg("%bl", "NULL", EBX);
	X86ByteRegs[ECX] = CreateReg("%cl", "NULL", ECX);
	X86ByteRegs[EDX] = CreateReg("%dl", "NULL", EDX);
}
void PutASMCode(int code, Symbol opds[])
{
	/**
		For example:
			TEMPLATE(X86_JMP,      "jmp %0")
		If code is X86_JMP,
			fmt is "jmp %0"
	 */
	char *fmt = ASMTemplate[code];
	int i;
	
	PutChar('\t');
	while (*fmt)
	{
		switch (*fmt)
		{
		case ';':
			PutString("\n\t");
			break;

		case '%':	
			// 	Linux:
			// TEMPLATE(X86_MOVI4,    "movl %1, %0")
			fmt++;
			if (*fmt == '%')
			{
				PutChar('%');
			}
			else
			{
				i = *fmt - '0';
				if (opds[i]->reg != NULL)
				{
					PutString(opds[i]->reg->name);
				}
				else
				{
					PutString(GetAccessName(opds[i]));
				}
			}
			break;

		default:
			PutChar(*fmt);
			break;
		}
		fmt++;
	}
	PutChar('\n');		
}

void DefineLabel(Symbol p)
{
	Print("%s:\n", GetAccessName(p));
}
void DefineCommData(Symbol p)
{
	GetAccessName(p);
	if (p->sclass == TK_STATIC)
	{
		/**
			#include <stdio.h>
			int a = 3;
			static int b;
		 */
		Print(".lcomm\t%s,%d\n", p->aname, p->ty->size);
	}
	else
	{
		Print(".comm\t%s,%d\n", p->aname, p->ty->size);
	}
}
void EndProgram(void)
{
	Flush();
}
