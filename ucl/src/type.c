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
static Type DoTypeClone(Type ty)
{
	int categ = ty->categ;
	if (categ == STRUCT) {
		;
	} else {
		Type qty;
		CALLOC(qty);
		*qty = *ty;
		return qty;
	}
}
Type Qualify(int qual, Type ty)
{
	Type qty;
	if (qual == 0 || qual == ty->qual)
		return ty;
	qty = DoTypeClone(ty);
	qty->qual |= qual;
	if (ty->qual != 0) {
		qty->bty = ty->bty;
	} else {
		qty->bty = ty;
	}
	return qty;
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
		T(i)->align = T(i)->size;
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

/**
 * Add a field to struct/union type ty. fty is the type of the field.
 * id is the name of the field. If the field is a bit-field, bits is its
 * bit width, for non bit-field, bits will be 0.
 */
/**
	struct Data{
		int a:2;		------------>   Field
		......
	}
		@ty		is a object of RecordType, the type of "Data"
		@id		field name,	"a"
		@fty	the type of struct field	, "int"
		@bits	whether this field is bit field, if yes, @bits is the length of bits, "2"
 */
Field AddField(Type ty, char *id, Type fty)
{
	RecordType rty = (RecordType)ty;
	Field fld;

	ALLOC(fld);
	fld->id = id;
	fld->ty = fty;
	fld->pos = fld->offset = 0;
	fld->next = NULL;
	
	*rty->tail = fld;
	rty->tail = &(fld->next);

	return fld;
}
Field LookupField(Type ty, char *id)
{
	RecordType rty = (RecordType)ty;
	Field fld = rty->flds;
	while (fld) {
		if (fld->id == id)
			return fld;
		fld = fld->next;
	}
	return NULL;
}
/**
 * Construct a struct/union type whose name is id, id can be NULL.
 */
Type StartRecord(char *id, int categ)
{
	RecordType rty;
	ALLOC(rty);

	memset(rty, 0, sizeof(*rty));
	rty->categ = categ;
	rty->id = id;
	rty->tail = &rty->flds;
	
	rty->complete = 0;
	return (Type)rty;
}

/**
 * When a struct/union type's delcaration is finished, layout the struct/union.
 * Calculate each field's offset from the beginning of struct/union and the size
 * and alignment of struct/union.
 */
void EndRecord(Type ty)
{
	RecordType rty = (RecordType)ty;
	Field fld = rty->flds;
	if (rty->categ == STRUCT) {
		/**
			At first, rty->size is 0.  
					rty->align is 0.
				See function StartRecord().
		 */
		while (fld) {
			fld->offset = rty->size = ALIGN(rty->size, fld->ty->align);
			/*
			if (fld->id == NULL && IsRecordType(fld->ty))
			{
				AddOffset((RecordType)fld->ty, fld->offset);
			}	
			*/
			rty->size = rty->size + fld->ty->size;		
			if (fld->ty->align > rty->align) {
				rty->align = fld->ty->align;
			}
			fld = fld->next;
		}
		rty->size = ALIGN(rty->size, rty->align);
	}
}
Type CommonRealType(Type ty1, Type ty2)
{
	ty1 = ty1->categ < INT ? T(INT) : ty1;
	ty2 = ty2->categ < INT ? T(INT) : ty2;
	if (ty1->categ == ty2->categ)
		return ty1;
	// ty1 and ty2 have the same sign
	// if ((IsUnsigned(ty1) ^ IsUnsigned(ty2)) == 0)
	// 	return ty1->categ > ty2->categ ? ty1 : ty2;

	// Their signs are different.
	// Swap ty1 and ty2, then we treat ty1 as Unsigned, ty2 as signed later.
	// if (IsUnsigned(ty2))
	// {
	// 	Type ty;

	// 	ty = ty1;
	// 	ty1 = ty2;
	// 	ty2 = ty;
	// }		
	// fg: (ty1,ty2) : ( ULONG,INT)
	if (ty1->categ  >= ty2->categ)
		return ty1;
	// fg: (ty1,ty2) : ( UINT, LONG)
	/**
		if the size of UINT and LONG are both 4 bytes,
			we will return ULONG as the common real type later.
		If signed long is large enough to accommodate UINT,
			return LONG.
	 */
	if (ty2->size > ty1->size)
		return ty2;

	return T(ty2->categ + 1);
}