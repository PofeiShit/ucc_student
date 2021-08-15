#ifndef __SYMBOL_H_
#define __SYMBOL_H_

enum 
{ 
	SK_Tag, SK_Temp, SK_String, SK_Label, SK_Constant, SK_Variable, SK_Function, SK_Register, SK_IRegister,
};

#define SYM_HASH_MASK 127
#define SYMBOL_COMMON    \
    int kind;            \
    char *name;          \
    char *aname;         \
	Type ty;			 \
    int level;           \
	int sclass;			 \
    union value val;     \
    struct symbol *reg;  \
    struct symbol *link; \
    struct symbol *next; \

typedef struct bblock *BBlock;

typedef struct symbol
{
	SYMBOL_COMMON
} *Symbol;

typedef struct variableSymbol
{
	SYMBOL_COMMON
	// InitData idata;
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
