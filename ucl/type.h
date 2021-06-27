#ifndef __TYPE_H_
#define __TYPE_H_

enum 
{
	CHAR, INT, POINTER, VOID, ARRAY, FUNCTION
};

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

typedef struct functionType
{
	TYPE_COMMON
} *FunctionType;

#define T(categ) (Types + categ)
extern struct type Types[VOID - CHAR + 1];
extern Type DefaultFunctionType;
#endif
