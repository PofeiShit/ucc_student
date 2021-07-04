#include "ucl.h"
#include "output.h"

// number of strings
static int StringNum;
static Symbol *FunctionTail, *StringTail, *GlobalTail;
// number of temporaries
int TempNum;

int LabelNum;
Symbol Strings;
Symbol Functions;
Symbol Globals;
/**
	Lookup a const first, 
	if not existing, then add a new one.

	Constant:
			int, pointer, float,double
 */

void InitSymbolTable()
{
	Globals = Functions = Strings = NULL;
	FunctionTail = &Functions;
	StringTail = &Strings;
	GlobalTail = &Globals;
	TempNum = LabelNum = StringNum = 0;	
}
Symbol AddVariable(char *name, Type ty)
{
	VariableSymbol p;
	CALLOC(p);
	p->kind = SK_Variable;
	p->name = name;
	p->ty = ty;
	*GlobalTail = (Symbol)p;
	GlobalTail = &p->next;

	*FSYM->lastv = (Symbol)p;
	FSYM->lastv = &p->next;
	return (Symbol)p;
}
Symbol AddFunction(char *name, Type ty)
{
	FunctionSymbol p;
	CALLOC(p);
	p->kind = SK_Function;
	p->name = name;
	p->ty = ty;
	p->lastv = &p->params;
	*FunctionTail = (Symbol)p;
	FunctionTail = &p->next;

	return (Symbol)p;
}

Symbol AddConstant(Type ty, union value val)
{
	// unsigned h = (unsigned)val.i[0] & SYM_HASH_MASK;
	Symbol p;

	// If not existing, we will create a new one.
	CALLOC(p);

	p->kind = SK_Constant;
	switch(ty->categ) {
	case INT:
		p->name = FormatName("%d", val.i[0]);
		break;
	default:
		;
	}
	p->ty = ty;
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

	return AddConstant(T(INT), val);
}
Symbol AddString(Type ty, String str)
{
	Symbol p;
	CALLOC(p);
	p->kind = SK_String;
	p->name = FormatName("str%d", StringNum++);
	p->val.p = str;
	p->ty = ty;	
	
	*StringTail = p;
	StringTail = &p->next;
	return p;
}
/**
	the temporay variable's name in a function is  t1,t2,....
 */
Symbol CreateTemp(Type ty)
{
	VariableSymbol p;

	CALLOC(p);

	p->kind = SK_Temp;
	p->name = FormatName("t%d", TempNum++);
	p->ty = ty;
	// p->pcoord = FSYM->pcoord;
	*FSYM->lastv = (Symbol)p;
	FSYM->lastv = &p->next;
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
