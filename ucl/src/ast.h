#ifndef __AST_H_
#define __AST_H_


enum nodeKind 
{ 
	NK_TranslationUnit, NK_Specifiers, NK_Token, NK_StructSpecifier, NK_StructDeclaration, NK_StructDeclarator, 
	NK_Function, NK_Declaration,
	NK_FunctionDeclarator, NK_ParameterTypeList, NK_ParameterDeclaration, NK_NameDeclarator,

	NK_Expression, 

	NK_ExpressionStatement, NK_ReturnStatement, NK_CompoundStatement,
};
typedef struct astExpression *AstExpression;
typedef struct astStatement *AstStatement;
typedef struct astDeclaration *AstDeclaration;
typedef struct astTranslationUnit *AstTranslationUnit;

#define AST_NODE_COMMON   \
    int kind;             \
    struct astNode *next; 

typedef struct astNode
{
	AST_NODE_COMMON
} *AstNode;

#define CREATE_AST_NODE(p, k) \
    CALLOC(p);                \
    p->kind = NK_##k;         

#define NEXT_TOKEN  CurrentToken = GetNextToken();

AstStatement       ParseCompoundStatement(void);
AstExpression      ParseExpression(void);
AstDeclaration     ParseDeclaration(void);
AstExpression      ParseAssignmentExpression(void);
AstTranslationUnit ParseTranslationUnit(char *file);

void CheckTranslationUnit(AstTranslationUnit transUnit);
void Translate(AstTranslationUnit transUnit);
void EmitTranslationUnit(AstTranslationUnit transUnit);
extern int CurFileLineNo;
extern const char *CurFileName;
void Do_Expect(int tok);
#define Expect CurFileName = __FILE__, CurFileLineNo = __LINE__, Do_Expect

extern int CurrentToken;
int  CurrentTokenIn(int toks[]);
extern int FIRST_Declaration[];
#endif