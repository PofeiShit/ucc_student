#include "ucl.h"
#include "output.h"

typedef struct  bucketLinker{
	struct bucketLinker * link;
	Symbol sym;
} * BucketLinker;

// number of strings
static int StringNum;
// tags in global scope, tag means struct/union, enumeration name
static struct table GlobalTags;
// normal identifiers in global scope
static struct table GlobalIDs;
// all the constants
static struct table Constants;
// tags in current scope
static Table Tags;
// normal identifiers in current scope
static Table Identifiers;
static int inParameterList = 0;
static Table savedIdentifiers, savedTags;

int IsInParameterList(void){
	return inParameterList;
}

void EnterParameterList(void){
	inParameterList = 1;
	EnterScope();	
}
void LeaveParemeterList(void){
	inParameterList = 0;
	ExitScope();
}
void SaveParameterListTable(void){
	savedIdentifiers = Identifiers;
	savedTags = Tags;
}
void RestoreParameterListTable(void){
	Level++;
	savedIdentifiers->outer = Identifiers;
	savedIdentifiers->level = Level;
	Identifiers = savedIdentifiers;

	savedTags->outer = Tags;
	savedTags->level = Level;
	Tags = savedTags;
}


static Symbol *FunctionTail, *StringTail, *GlobalTail;
/// Scope level, file scope will be 0, when entering each nesting level,
/// Level increment; exiting each nesting level, Level decrement
int Level;
// number of temporaries
int TempNum;

int LabelNum;
Symbol Strings;
Symbol Functions;
Symbol Globals;
#define	SEARCH_OUTER_TABLE	1

static Symbol DoLookupSymbol(Table tbl, char *name, int  searchOuter){
	Symbol p;
	/**
		h is used as key to search the hashtable.
	 */
	unsigned h = (unsigned long)name & SYM_HASH_MASK;
	BucketLinker linker;
	do{
		if (tbl->buckets != NULL){
			for (linker =(BucketLinker) tbl->buckets[h]; linker; linker = linker->link)	{
				if (linker->sym->name == name){
					linker->sym->level = tbl->level;
					return  linker->sym;
				}
			}
		}
	} while ((tbl = tbl->outer) != NULL && searchOuter);
	return NULL;	
}

static Symbol LookupSymbol(Table tbl, char *name)
{
	return DoLookupSymbol(tbl, name, SEARCH_OUTER_TABLE);
}

/**
 * Add a symbol sym to symbol table tbl
 */
static Symbol AddSymbol(Table tbl, Symbol sym)
{
	unsigned int h = (unsigned long)sym->name & SYM_HASH_MASK;
	BucketLinker  linker;
	CALLOC(linker);
	if (tbl->buckets == NULL)
	{
		int size = sizeof(Symbol) * (SYM_HASH_MASK + 1);

		tbl->buckets = HeapAllocate(CurrentHeap, size);
		memset(tbl->buckets, 0, size);
	}
	// add the new symbol into the first positon of bucket[h] list.
	linker->link = (BucketLinker) tbl->buckets[h];
	linker->sym = sym;
	sym->level = tbl->level;
	tbl->buckets[h] = (Symbol) linker;;
	return sym;
}

/**
 * Enter a nesting scope. Increment the nesting level and 
 * create two new symbol table for normal identifiers and tags.
 */
void EnterScope(void)
{
	Table t;

	Level++;

	ALLOC(t);
	t->level = Level;
	t->outer = Identifiers;
	t->buckets = NULL;
	Identifiers = t;

	ALLOC(t);
	t->level = Level;
	t->outer = Tags;
	t->buckets = NULL;
	Tags = t;
}

/**
 * Exit a nesting scope. Decrement the nesting level and 
 * up to the enclosing normal identifiers and tags
 */
void ExitScope(void)
{
	Level--;
	Identifiers = Identifiers->outer;
	Tags = Tags->outer;
}

Symbol LookupID(char *name)
{
	return LookupSymbol(Identifiers, name);
}

Symbol LookupTag(char *name)
{
	return LookupSymbol(Tags, name);
}
/**
	Lookup a const first, 
	if not existing, then add a new one.

	Constant:
			int, pointer, float,double
 */
Symbol AddTag(char *name, Type ty)
{
	Symbol p;
	ALLOC(p);
	p->kind = SK_Tag;
	p->name = name;
	p->ty = ty;
	
	return AddSymbol(Tags, p);
}

void InitSymbolTable()
{
	Level = 0;
	//	Hashtable	data-structure
	GlobalTags.buckets = GlobalIDs.buckets = NULL;
	GlobalTags.outer = GlobalIDs.outer = NULL;
	GlobalTags.level = GlobalIDs.level = 0;
	Globals = Functions = Strings = NULL;
	FunctionTail = &Functions;
	StringTail = &Strings;
	GlobalTail = &Globals;

	Tags = &GlobalTags;
	Identifiers = &GlobalIDs;
	TempNum = LabelNum = StringNum = 0;	
}
Symbol AddVariable(char *name, Type ty, int sclass)
{
	VariableSymbol p;
	CALLOC(p);
	p->kind = SK_Variable;
	p->name = name;
	p->ty = ty;
	p->sclass = sclass;
	if (Level == 0 || sclass == TK_STATIC)
	{
		*GlobalTail = (Symbol)p;
		GlobalTail = &p->next;
	}
	else if (sclass != TK_EXTERN)
	{
		*FSYM->lastv = (Symbol)p;
		FSYM->lastv = &p->next;
	}
	if(sclass == TK_EXTERN  && Identifiers  != &GlobalIDs){
		AddSymbol(&GlobalIDs, (Symbol)p);		
	}
	return AddSymbol(Identifiers, (Symbol)p);
}

Symbol AddFunction(char *name, Type ty, int sclass)
{
	FunctionSymbol p;
	CALLOC(p);
	p->kind = SK_Function;
	p->name = name;
	p->ty = ty;
	p->sclass = sclass;
	p->lastv = &p->params;
	*FunctionTail = (Symbol)p;
	FunctionTail = &p->next;
	if(Identifiers  != &GlobalIDs){
		AddSymbol(Identifiers, (Symbol)p);		
	}

	return AddSymbol(&GlobalIDs, (Symbol)p);


}

Symbol AddConstant(Type ty, union value val)
{
	// unsigned h = (unsigned)val.i[0] & SYM_HASH_MASK;
	Symbol p;
	
	if (IsIntegType(ty)) {
		ty = T(INT);
	}
	// If not existing, we will create a new one.
	CALLOC(p);

	p->kind = SK_Constant;
	switch(ty->categ) 
	{
		case INT:
			p->name = FormatName("%d", val.i[0]);
			break;
		default:
			;
	}
	// p->pcoord = FSYM->pcoord;

	p->ty = ty;
	//p->sclass = TK_STATIC;
	p->val = val;
	// insert the new const into hashtable bucket.
	// p->link = Constants.buckets[h];
	// Constants.buckets[h] = p;
	// if it is a float const, added to end of FloatConsts linked-list.

	return p;
}
Symbol IntConstant(int i)
{
	union value val;

	val.i[0] = i;
	val.i[1] = 0;

	return AddConstant(T(INT), val);
}
Symbol AddString(Type ty, String str)
{
	Symbol p;
	CALLOC(p);
	p->kind = SK_String;
	p->name = FormatName("str%d", StringNum++);
	p->val.p = str;
	p->ty = ty;	
	
	*StringTail = p;
	StringTail = &p->next;
	return p;
}
/**
	the temporay variable's name in a function is  t1,t2,....
 */
Symbol CreateTemp(Type ty)
{
	VariableSymbol p;

	CALLOC(p);

	p->kind = SK_Temp;
	p->name = FormatName("t%d", TempNum++);
	p->ty = ty;
	p->level = 1;
	// p->pcoord = FSYM->pcoord;
	*FSYM->lastv = (Symbol)p;
	FSYM->lastv = &p->next;
	printf("temp %s %s %d %d\n", FSYM->name, p->name, p, ty->size);
	return (Symbol)p;
}
// mainly create basic block's label name.
Symbol CreateLabel(void)
{
	Symbol p;

	CALLOC(p);

	p->kind = SK_Label;
	p->name = FormatName("BB%d", LabelNum++);
	
	return p;
}

