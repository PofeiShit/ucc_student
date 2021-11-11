#ifndef __TYPE_H_
#define __TYPE_H_

enum 
{
	CHAR, UCHAR, SHORT, USHORT, INT, UINT, ENUM, POINTER, VOID, STRUCT, ARRAY, FUNCTION
};
enum {CONST=0x1, VOLATILE=0x2};
enum {I1, U1, I2, U2, I4, U4, V, B};

#define TYPE_COMMON \
	int categ : 8; \
	int qual : 8; \
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
#define IsUnsigned(ty) (ty->categ & 0x1)
#define IsScalarType(ty) (ty->categ <= POINTER)
#define IsPtrType(ty) (ty->categ == POINTER)
#define IsFunctionType(ty) (ty->categ == FUNCTION)
#define IsObjectPtr(ty) (ty->categ == POINTER && ty->bty->categ != FUNCTION)
#define IsArithType(ty)    (ty->categ <= ENUM)
#define BothIntegType(ty1, ty2)   (IsIntegType(ty1) && IsIntegType(ty2))
#define BothArithType(ty1, ty2)   (IsArithType(ty1) && IsArithType(ty2))
Field LookupField(Type ty, char *id);
Type  StartRecord(char *id, int categ);
Field AddField(Type ty, char *id, Type fty);
void EndRecord(Type ty);
Type PointerTo(Type ty);
Type Unqual(Type ty);
Type Qualify(int qual, Type ty);
Type ArrayOf(int len, Type ty);
Type FunctionReturn(Type ty, Signature sig);
Type Promote(Type ty);

Type CommonRealType(Type ty1, Type ty2);
int TypeCode(Type ty);
Type AdjustParameter(Type ty);

void SetupTypeSystem(void);
extern struct type Types[VOID - CHAR + 1];
extern Type DefaultFunctionType;
#endif
