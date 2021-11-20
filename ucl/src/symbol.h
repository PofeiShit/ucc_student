#ifndef __SYMBOL_H_
#define __SYMBOL_H_

enum 
{ 
	SK_Tag, SK_TypedefName, SK_Temp, SK_String, SK_Label, SK_Constant, SK_Variable, 
	SK_Function, SK_Register, SK_IRegister, SK_Offset, SK_EnumConstant,
};

#define SYM_HASH_MASK 127
#define SYMBOL_COMMON    \
    int kind;            \
    char *name;          \
    char *aname;         \
	Type ty;			 \
    int level;           \
	int sclass;			 \
	int ref;			 \
	int needwb : 1;		 \
    union value val;     \
    struct symbol *reg;  \
    struct symbol *link; \
    struct symbol *next; \

typedef struct bblock *BBlock;
typedef struct initData *InitData;
typedef struct symbol
{
	SYMBOL_COMMON
} *Symbol;

typedef struct variableSymbol
{
	SYMBOL_COMMON
	InitData idata;
	// ValueDef def;
	// ValueUse uses;
	int offset;
} *VariableSymbol;

typedef struct functionSymbol
{
	SYMBOL_COMMON
	Symbol params;
	Symbol locals;
	Symbol *lastv;
	//int nbblock;
	BBlock entryBB;
	BBlock exitBB;
} *FunctionSymbol;

typedef struct table
{
	Symbol *buckets;
	int level;
	struct table *outer;
} *Table;

void EnterScope(void);
void ExitScope(void);

#define AsVar(sym) ((VariableSymbol)sym)
#define AsFunc(sym) ((FunctionSymbol)sym)

Symbol AddEnumConstant(char *id, Type ty, int val);
Symbol AddConstant(Type ty, union value val);
Symbol AddVariable(char *id, Type ty, int sclass);
Symbol AddFunction(char *id, Type ty, int sclass);
Symbol AddTag(char *id, Type ty);
Symbol IntConstant(int i);
Symbol CreateTemp(Type ty);
Symbol CreateLabel(void);
Symbol CreateOffset(Type ty, Symbol base, int coff);
Symbol AddString(Type ty, String str);
Symbol AddTypedefName(char *name, Type ty);
Symbol LookupID(char *id);
Symbol LookupTag(char *id);
void InitSymbolTable(void);
void EnterParameterList(void);
void LeaveParemeterList(void);
void SaveParameterListTable(void);
void RestoreParameterListTable(void);

extern int Level;
extern int TempNum;
extern Symbol Strings;
extern Symbol Globals;
extern FunctionSymbol FSYM;
#endif
