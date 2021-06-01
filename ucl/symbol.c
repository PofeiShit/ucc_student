#include "ucl.h"
#include "output.h"

// number of strings
static int StringNum;
static Symbol *FunctionTail, *StringTail;
// number of temporaries
int TempNum;

int LabelNum;
Symbol Strings;
Symbol Functions;
/**
	Lookup a const first, 
	if not existing, then add a new one.

	Constant:
			int, pointer, float,double
 */

void InitSymbolTable()
{
	Functions = Strings = NULL;
	FunctionTail = &Functions;
	StringTail = &Strings;
	TempNum = LabelNum = StringNum = 0;	
}
Symbol AddFunction(char *name)
{
	FunctionSymbol p;
	CALLOC(p);
	p->kind = SK_Function;
	p->name = name;
	*FunctionTail = (Symbol)p;
	FunctionTail = &p->next;

	return (Symbol)p;
}

Symbol AddConstant(union value val)
{
	// unsigned h = (unsigned)val.i[0] & SYM_HASH_MASK;
	Symbol p;

	// If not existing, we will create a new one.
	CALLOC(p);

	p->kind = SK_Constant;
	p->name = FormatName("%d", val.i[0]);

	// p->pcoord = FSYM->pcoord;

	// p->ty = ty;
	//p->sclass = TK_STATIC;
	p->val = val;
	// insert the new const into hashtable bucket.
	// p->link = Constants.buckets[h];
	// Constants.buckets[h] = p;
	// if it is a float const, added to end of FloatConsts linked-list.

	return p;
}
Symbol IntConstant(int i)
{
	union value val;

	val.i[0] = i;
	val.i[1] = 0;

	return AddConstant(val);
}
Symbol AddString(String str)
{
	Symbol p;
	CALLOC(p);
	p->kind = SK_String;
	p->name = FormatName("str%d", StringNum++);
	p->val.p = str;
	
	*StringTail = p;
	StringTail = &p->next;
	return p;
}
/**
	the temporay variable's name in a function is  t1,t2,....
 */
Symbol CreateTemp()
{
	VariableSymbol p;

	CALLOC(p);

	p->kind = SK_Temp;
	p->name = FormatName("t%d", TempNum++);

	// p->pcoord = FSYM->pcoord;
	//*FSYM->lastv = (Symbol)p;
	//FSYM->lastv = &p->next;
	return (Symbol)p;
}
// mainly create basic block's label name.
Symbol CreateLabel(void)
{
	Symbol p;

	CALLOC(p);

	p->kind = SK_Label;
	p->name = FormatName("BB%d", LabelNum++);
	
	return p;
}
