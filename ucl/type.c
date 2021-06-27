#include "ucl.h"
#include "output.h"
#include "type.h"
#include "config.h"
struct type Types[VOID - CHAR + 1];
Type DefaultFunctionType;

Type ArrayOf(int len, Type ty)
{
	ArrayType aty;
	CALLOC(aty);
	aty->categ = ARRAY;
	aty->size = len * ty->size;
	aty->bty = ty;
	return (Type)aty;
}
Type FunctionReturn(Type ty)
{
	FunctionType fty;
	ALLOC(fty);

	fty->categ = FUNCTION;
	fty->size = T(POINTER)->size;
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
	DefaultFunctionType = (Type)fty;
}

