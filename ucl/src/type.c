#include "ucl.h"
#include "output.h"
#include "type.h"
#include "config.h"
struct type Types[VOID - CHAR + 1];
Type DefaultFunctionType;

int TypeCode(Type ty)
{
	// CHAR, UCHAR, SHORT, USHORT, INT, UINT, ENUM, POINTER, VOID, STRUCT, ARRAY, FUNCTION
	static int optypes[] = {I1, U1, I2, U2, I4, U4, I4, U4, V, B, B, B};
	return optypes[ty->categ];
}
static const char * categNames[] = {                                                                 
     "CHAR", "UCHAR", "SHORT", "USHORT", "INT", "UINT", "ENUM",       
     "POINTER", "VOID", "STRUCT","ARRAY", "FUNCTION","NA"   
};  
const char * GetCategName(int categ){                                                                
	return categNames[categ]; 
}  
Type Unqual(Type ty)
{
	if (ty->qual)
		ty = ty->bty;
	return ty;
}
Type PointerTo(Type ty)
{
	Type pty;
	ALLOC(pty);

	pty->categ = POINTER;
	pty->qual = 0;
	pty->align = T(POINTER)->align;
	pty->size = T(POINTER)->size;
	pty->bty = ty;
	return pty;
}
static Type DoTypeClone(Type ty)
{
	int categ = ty->categ;
	if (categ == STRUCT) {
		;
	} else if (categ == ARRAY) {
		ArrayType aty;
		CALLOC(aty);
		*aty = *((ArrayType)ty);
		return (Type)aty;
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
	aty->align = ty->align;
	aty->bty = ty;
	return (Type)aty;
}
Type FunctionReturn(Type ty, Signature sig)
{
	FunctionType fty;
	ALLOC(fty);

	fty->categ = FUNCTION;
	fty->size = T(POINTER)->size;
	fty->align = T(POINTER)->align;
	fty->sig = sig;
	fty->bty = ty;
	return (Type)fty;
}
Type Enum(char *id)
{
	EnumType ety;
	ALLOC(ety);
	ety->categ = ENUM;
	ety->id = id;

	ety->bty = T(INT);
	ety->size = ety->bty->size;
	ety->align = ety->bty->align;
	ety->qual = 0;
	return (Type)ety;
}
void SetupTypeSystem(void)
{
	int i;
	FunctionType fty;
	T(CHAR)->size = T(UCHAR)->size = CHAR_SIZE;
	T(SHORT)->size = T(USHORT)->size = SHORT_SIZE;
	T(INT)->size = T(UINT)->size = INT_SIZE;
	T(POINTER)->size = INT_SIZE;
	T(POINTER)->bty = T(INT);

	for (i = CHAR; i <= VOID; ++i)
	{
		T(i)->categ = i;
		T(i)->align = T(i)->size;
	}

	ALLOC(fty);
	fty->categ = FUNCTION;
	fty->align = fty->size = T(POINTER)->size;
	fty->bty = T(INT);

	ALLOC(fty->sig);
	CALLOC(fty->sig->params);
	fty->sig->hasEllipsis = 0;

	DefaultFunctionType = (Type)fty;
}

Type AdjustParameter(Type ty)
{
	ty = Unqual(ty);
	if (ty->categ == ARRAY)
		return PointerTo(ty->bty);
	if (ty->categ == FUNCTION)
		return PointerTo(ty);
	return ty;
}
int IsZeroSizeArray(Type ty)
{
	ty = Unqual(ty);
	return (ty->categ == ARRAY && (((ArrayType)ty)->len == 0) && ty->size == 0);
}
int IsIncompleteEnum(Type ty)
{
	ty = Unqual(ty);
	return (ty->categ == ENUM && !((EnumType)ty)->complete);
}
int IsIncompleteRecord(Type ty)
{
	ty = Unqual(ty);
	return (IsRecordType(ty) && !((RecordType)ty)->complete);
}
int IsIncompleteType(Type ty, int ignoreZeroArray) 
{
	ty = Unqual(ty);
	switch(ty->categ)
	{
		case ENUM:
			return IsIncompleteEnum(ty);
		case STRUCT:
			return IsIncompleteRecord(ty);
		case ARRAY:
			if (ignoreZeroArray) {
				return IsIncompleteType(ty->bty, IGNORE_ZERO_SIZE_ARRAY);
			} else {
				if (IsZeroSizeArray(ty))
					return 1;
				else
					return IsIncompleteType(ty->bty, !IGNORE_ZERO_SIZE_ARRAY);
			}
		default:
			return 0;
	}
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
	if (fty->size == 0) {
		if (fty->categ == ARRAY) {
			rty->hasFlexArray = 1;
		}
	}
	if (fty->qual & CONST) {
		rty->hasConstFld = 1;
	}
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
	while (fld != NULL) {
		// struct Data {
		// 	struct {
		// 		int a;
		// 		int b;
		// 	};
		// 	int c;
		// };
		// struct Data dt;
		// dt.b;
		if (fld->id == NULL && IsRecordType(fld->ty)) {
			Field p;
			p = LookupField(fld->ty, id);
			if (p) {
				return p;
			}
		} else if (fld->id == id)
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
void AddOffset(RecordType rty, int offset)
{
	Field fld = rty->flds;
	while (fld) {
		fld->offset += offset;
		if (fld->id == NULL && IsRecordType(fld->ty)) {
			AddOffset((RecordType)fld->ty, fld->offset);
		}
		fld = fld->next;
	}
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
			if (fld->id == NULL && IsRecordType(fld->ty))
			{
				AddOffset((RecordType)fld->ty, fld->offset);
			}	
			rty->size = rty->size + fld->ty->size;		
			if (fld->ty->align > rty->align) {
				rty->align = fld->ty->align;
			}
			fld = fld->next;
		}
		rty->size = ALIGN(rty->size, rty->align);
	}
	// struct Buffer {
		// char buf[];
	// }
	if (rty->categ == STRUCT && rty->size == 0 && rty->hasFlexArray) {
		Error(NULL, "flexible array member in otherwise empty struct");
	}
}
Type CommonRealType(Type ty1, Type ty2)
{
	ty1 = ty1->categ < INT ? T(INT) : ty1;
	ty2 = ty2->categ < INT ? T(INT) : ty2;
	if (ty1->categ == ty2->categ)
		return ty1;
	// ty1 and ty2 have the same sign
	if ((IsUnsigned(ty1) ^ IsUnsigned(ty2)) == 0)
		return ty1->categ > ty2->categ ? ty1 : ty2;

	// Their signs are different.
	// Swap ty1 and ty2, then we treat ty1 as Unsigned, ty2 as signed later.
	if (IsUnsigned(ty2))
	{
		Type ty;

		ty = ty1;
		ty1 = ty2;
		ty2 = ty;
	}		
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
static int IsCompatibleFunction(FunctionType fty1, FunctionType fty2)
{
	Signature sig1 = fty1->sig;
	Signature sig2 = fty2->sig;
	Parameter p1, p2;
	int parLen1, parLen2;
	int i;
	if (!IsCompatibleType(fty1->bty, fty2->bty)) {
		return 0;
	}
	parLen1 = LEN(sig1->params);
	parLen2 = LEN(sig2->params);
	if ((sig1->hasEllipsis ^ sig2->hasEllipsis) || parLen1 != parLen2) {
		return 0;
	}
	for (i = 0; i < parLen1; i++) {
		p1 = (Parameter)GET_ITEM(sig1->params, i);
		p2 = (Parameter)GET_ITEM(sig2->params, i);
		if (!IsCompatibleType(p1->ty, p2->ty)) {
			return 0;
		}
	}
	return 1;
}
int IsCompatibleType(Type ty1, Type ty2)
{
	if (ty1 == ty2)
		return 1;
	if (ty1->qual != ty2->qual)
		return 0;
	ty1 = Unqual(ty1);
	ty2 = Unqual(ty2);
	if (ty1->categ != ty2->categ)
		return 0;
	
	switch(ty1->categ)
	{
		case POINTER:
			return IsCompatibleType(ty1->bty, ty2->bty);
		case ARRAY:
			return IsCompatibleType(ty1->bty, ty2->bty) && (ty1->size == ty2->size || ty1->size == 0 || ty2->size == 0);
		case FUNCTION:
			return IsCompatibleFunction((FunctionType)ty1, (FunctionType)ty2);
		default:
			return ty1 == ty2;
	}
}

Type CompositeType(Type ty1, Type ty2)
{
	if (ty1->categ == ENUM) 
		return ty1;
	if (ty2->categ == ENUM)
		return ty2;
	switch(ty1->categ) {
	case POINTER:
		return Qualify(ty1->qual, PointerTo(CompositeType(ty1->bty, ty2->bty)));
	case ARRAY:
		return ty1->size != 0 ? ty1 : ty2;
	case FUNCTION:
		{
			FunctionType fty1 = (FunctionType)ty1;
			FunctionType fty2 = (FunctionType)ty2;
			fty1->bty = CompositeType(fty1->bty, fty2->bty);

			Parameter p1, p2;
			int i, len = LEN(fty1->sig->params);

			for (int i = 0; i < len; i++) {
				p1 = (Parameter)GET_ITEM(fty1->sig->params, i);
				p2 = (Parameter)GET_ITEM(fty2->sig->params, i);
				p1->ty = CompositeType(p1->ty, p2->ty);
			}
			return ty1;
		}
	default:
		return ty1;
	}
}