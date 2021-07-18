#ifndef __TYPE_H_
#define __TYPE_H_

enum 
{
	CHAR, INT, ENUM, POINTER, VOID, ARRAY, FUNCTION
};
enum {I1, I4};

#define TYPE_COMMON \
	int categ : 8; \
	int size; \
	struct type *bty;

typedef struct type
{
	TYPE_COMMON
} *Type;

typedef struct arrayType
{
	TYPE_COMMON
	int len;
} *ArrayType;

typedef struct parameter
{
	char *id;
	Type ty;
	int reg;
} *Parameter;

typedef struct signature
{
	int hasProto : 16;
	int hasEllipsis : 16;
	Vector params;
} *Signature;

typedef struct functionType
{
	TYPE_COMMON
	Signature sig;
} *FunctionType;

#define T(categ) (Types + categ)

#define IsIntegType(ty) (ty->categ <= ENUM)
#define IsPtrType(ty) (ty->categ == POINTER)
#define IsFunctionType(ty) (ty->categ == FUNCTION)
extern int TypeCode(Type ty);
extern struct type Types[VOID - CHAR + 1];
extern Type DefaultFunctionType;
#endif
