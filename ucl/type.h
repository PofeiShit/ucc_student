#ifndef __TYPE_H_
#define __TYPE_H_

enum 
{
	CHAR, INT, ENUM, POINTER, VOID, STRUCT, ARRAY, FUNCTION
};
enum {I1, I4};

#define TYPE_COMMON \
	int categ : 8; \
	int align : 16; \
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

typedef struct field
{
	int offset;
	char *id;
	int pos;
	Type ty;
	struct field *next;
} *Field;
/**
	id:	struct/union name, for anonymous struct/union, id is NULL.
	flds: all the fields of struct/union
	tail:		used for construction of field list
	hasConstFld:	contains constant filed or not
	hasFlexArray:	contains flexible array(array with size 0) or not,
					the flexible array must be last field.
 */
typedef struct recordType
{
	TYPE_COMMON
	char *id; 
	Field flds; 
	Field *tail; 
	int hasConstFld : 16;
	int hasFlexArray : 16;
	//for test whether it is incomplete type.
	/**
		struct A;	//	It is considered an incomplete type here. So sizeof(struct A) is illegal.
		void f(int a[]){
			printf("%d \n",sizeof(a));
			printf("sizeof(struct A) = %d  \n",sizeof(struct A));------- incomplete type
		}
		struct A{
			int a[10];
		};
		see IsIncompleteType(ty)
	 */
	int complete;			//	

} *RecordType;

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
