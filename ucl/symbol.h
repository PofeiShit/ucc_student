#ifndef __SYMBOL_H_
#define __SYMBOL_H_

enum 
{ 
	SK_Temp, SK_String, SK_Label, SK_Constant, SK_Variable, SK_Function, SK_Register, SK_IRegister,
};


#define SYMBOL_COMMON    \
    int kind;            \
    char *name;          \
    char *aname;         \
	Type ty;			 \
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
	// int offset;
} *VariableSymbol;

typedef struct functionSymbol
{
	SYMBOL_COMMON
	//Symbol params;
	//Symbol locals;
	//Symbol *lastv;
	//int nbblock;
	BBlock entryBB;
	BBlock exitBB;
} *FunctionSymbol;
void InitSymbolTable(void);
extern Symbol Strings;
extern Symbol Globals;
extern FunctionSymbol FSYM;
#endif
