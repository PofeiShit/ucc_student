#include "ucl.h"
#include "output.h"
#include "type.h"
#include "config.h"
struct type Types[VOID - CHAR + 1];
Type DefaultFunctionType;

int TypeCode(Type ty)
{
	static int optypes[] = {I1, I4};
	return optypes[ty->categ];
}

Type PointerTo(Type ty)
{
	Type pty;
	ALLOC(pty);

	pty->categ = POINTER;
	pty->size = T(POINTER)->size;
	pty->bty = ty;
	return pty;
}

Type ArrayOf(int len, Type ty)
{
	ArrayType aty;
	CALLOC(aty);
	aty->categ = ARRAY;
	aty->size = len * ty->size;
	aty->bty = ty;
	return (Type)aty;
}
Type FunctionReturn(Type ty, Signature sig)
{
	FunctionType fty;
	ALLOC(fty);

	fty->categ = FUNCTION;
	fty->size = T(POINTER)->size;
	fty->sig = sig;
	fty->bty = ty;
	return (Type)fty;
}

void SetupTypeSystem(void)
{
	int i;
	FunctionType fty;
	T(CHAR)->size = CHAR_SIZE;
	T(INT)->size = INT_SIZE;
	T(POINTER)->size = INT_SIZE;
	T(POINTER)->bty = T(INT);

	for (i = CHAR; i <= VOID; ++i)
	{
		T(i)->categ = i;
	}

	ALLOC(fty);
	fty->categ = FUNCTION;
	fty->size = T(POINTER)->size;
	fty->bty = T(INT);

	ALLOC(fty->sig);
	CALLOC(fty->sig->params);
	fty->sig->hasProto = 0;
	fty->sig->hasEllipsis = 0;

	DefaultFunctionType = (Type)fty;
}

Type AdjustParameter(Type ty)
{
	if (ty->categ == FUNCTION)
		return PointerTo(ty);
	return ty;
}

Type Promote(Type ty)
{
	return ty->categ < INT ? T(INT) : ty;
}
