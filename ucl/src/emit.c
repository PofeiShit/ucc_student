#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "output.h"
#include "target.h"
int SwitchTableNum;
/**
 * Emit all the strings to assembly file
 */
static void EmitGlobals(void)
{
	Symbol p = Globals;
	InitData initd;
	int size, len;
	while (p)
	{
		initd = AsVar(p)->idata;
		if (p->sclass == TK_EXTERN && initd == NULL) {
			;
		} else if  (initd == NULL) {
			DefineCommData(p);
		} else {
			DefineGlobal(p);
			len = strlen(p->aname);
			size = 0;
			while (initd) {
				// printf("offset = %d ,size =  %d, val = %d\n",initd->offset,size, initd->expr->val.i[0]);
				// if (initd->offset != size) {
				// 	LeftAlign(ASMFile, len);
				// 	PutString("\t");
				// 	Space(initd->offset - size);
				// }
				if (initd->offset != 0) {
					LeftAlign(ASMFile, len);
					PutString("\t");
				}
				if (initd->expr->op == OP_ADD) {
					int n = initd->expr->kids[1]->val.i[0];
					DefineAddress((Symbol)initd->expr->kids[0]->val.p);
					PutString("\n");
				} else {
					DefineValue(initd->expr->ty, initd->expr->val);
				}
				size = initd->offset + initd->expr->ty->size;
				initd = initd->next;
			}
		}
		p = p->next;
	}
	PutString("\n");
}
static void EmitStrings(void)
{
	Symbol p = Strings;
	String str;
	int len;
	int size;
	while (p)
	{
		DefineGlobal(p);
		str = (String)p->val.p;
		//printf("%s\n", str->chs);
		//assert(p->ty->categ == ARRAY);
		size = str->len + 1;
		DefineString(str, size);
		p = p->next;
	}
	PutString("\n");
}

/**
 * Emit all the functions
 */
static void EmitFunctions(AstTranslationUnit transUnit)
{
	AstNode p;
	FunctionSymbol fsym;
	p = transUnit->extDecls;
	while (p != NULL)
	{
		
		if (p->kind == NK_Function)
		{
			fsym = ((AstFunction)p)->fsym;
			if (fsym->sclass != TK_STATIC || fsym->ref > 0)
				EmitFunction(fsym);
		}
		p = p->next;
	}
}

/**
 * Emit  assembly code for the translation unit
 */
void EmitTranslationUnit(AstTranslationUnit transUnit)
{
	if(ASMFileName){
		ASMFile = fopen(ASMFileName, "w");
		ASMFileName = NULL;
	}else{
		ASMFile = CreateOutput(Input.filename, ExtName);
	}
	SwitchTableNum = 1;
	// "# Code auto-generated by UCC\n\n"
	BeginProgram();
	// ".data\n\n"
	Segment(DATA);
	/**
		.str0:	.string	"%d \012"
		.str1:	.string	"a + b + c + d = %d.\012"
	 */
	EmitStrings();
	
	EmitGlobals();
	// ".text\n\n"
	Segment(CODE);

	/**
		The key function is 
			void EmitFunction(FunctionSymbol fsym)
		in x86.c
	 */
	EmitFunctions(transUnit);

	EndProgram();

	fclose(ASMFile);
}
