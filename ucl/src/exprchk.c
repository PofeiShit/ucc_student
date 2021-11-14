#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "decl.h"

#define SWAP_KIDS(expr)					\
{										\
     AstExpression t = expr->kids[0]; 	\
     expr->kids[0] = expr->kids[1];   	\
     expr->kids[1] = t;   				\
}

#define PERFORM_ARITH_CONVERSION(expr)                                 \
    expr->ty = CommonRealType(expr->kids[0]->ty, expr->kids[1]->ty);   \
    expr->kids[0] = Cast(expr->ty, expr->kids[0]);                     \
    expr->kids[1] = Cast(expr->ty, expr->kids[1]);

#define REPORT_OP_ERROR											\
	Error(NULL, "Invalid operands to %s", OPNames[expr->op]);	\
	expr->ty = T(INT);											\
	return expr;

static int CanModify(AstExpression expr)
{
	return (!(expr->ty->qual & CONST));
}
static AstExpression CastExpression(Type ty, AstExpression expr)
{
	AstExpression cast;
	if (expr->op == OP_CONST && ty->categ != VOID)
		return FoldCast(ty, expr);
	CREATE_AST_NODE(cast, Expression);
	cast->op = OP_CAST;
	cast->ty = ty;
	cast->kids[0] = expr;

	return cast;
}
AstExpression Cast(Type ty, AstExpression expr)
{
	int scode = TypeCode(expr->ty);
	int dcode = TypeCode(ty);
	// 两个类型是否都是占用相同大小内存的整型
	if (scode / 2 == dcode / 2) {
		int scateg = expr->ty->categ;
        int dcateg = ty->categ;
		if (scateg != dcateg && scateg >= INT && scateg <= UINT) {
			return CastExpression(ty, expr); // I4 与 U4 之间
		}
		// I1 和 U1 之间, I2 和 U2 之间
		expr->ty = ty;
		return expr;
	}
	if (scode != dcode) {
		if (dcode < I4) {
			expr = CastExpression(T(INT), expr);
		}
		expr = CastExpression(ty, expr);
	}
	return expr;
}

static AstExpression CheckPrimaryExpression(AstExpression expr)
{
	Symbol p;
	if (expr->op == OP_CONST) {
		return expr;
	}
	if (expr->op == OP_STR) {
		expr->op = OP_ID;
		expr->val.p = AddString(expr->ty, (String)expr->val.p);
		return expr;
	}	
	p = LookupID((char*)expr->val.p);
	{
		expr->ty = p->ty;
		expr->val.p = p;
	}
	return expr;
}

static AstExpression PromoteArgument(AstExpression arg)
{
	Type ty = Promote(arg->ty);
	return Cast(ty, arg);
}

AstExpression Adjust(AstExpression expr, int rvalue)
{
	int qual = 0;
	if (rvalue) {
		qual = expr->ty->qual;
		expr->ty = Unqual(expr->ty);
	}
	if (expr->ty->categ == FUNCTION) {
		expr->ty = PointerTo(expr->ty);
	} else if (expr->ty->categ == ARRAY) {
		expr->ty = PointerTo(Qualify(qual, expr->ty->bty));
		expr->isarray = 1;
	}
	return expr;
}

static AstExpression CheckArgument(FunctionType fty, AstExpression arg, int argNo, int *argFull)
{
	Parameter param;

	int parLen = LEN(fty->sig->params);
	arg = Adjust(CheckExpression(arg), 1);
	
	// f(void) 
	if (fty->sig->hasProto && parLen == 0)
	{
		*argFull = 1;
		return arg;
	}
	// f(int, int, int)  and check the last one parameter			
	if (argNo == parLen && ! fty->sig->hasEllipsis)
		*argFull = 1;

	// TODO:delete old style
	if (!fty->sig->hasProto) 
	{
		arg = PromoteArgument(arg);
		*argFull = 0;
		return arg;
	} else if (argNo <= parLen) {
		param = (Parameter)GET_ITEM(fty->sig->params, argNo - 1);
		// 
		if (param->ty->categ < INT)
			arg = Cast(T(INT), arg);
		else
			arg = Cast(param->ty, arg);
		return arg;
	}
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Type ty;
	AstNode *tail;
	int argNo, argFull;

	if (expr->kids[0]->op == OP_ID && LookupID((char*)expr->kids[0]->val.p) == NULL) {
		expr->kids[0]->ty = DefaultFunctionType;
		expr->kids[0]->val.p = AddFunction((char*)expr->kids[0]->val.p, DefaultFunctionType, TK_EXTERN);
	} else {
		expr->kids[0] = CheckExpression(expr->kids[0]);
	}
	expr->kids[0] = Adjust(expr->kids[0], 1);
	ty = expr->kids[0]->ty; // ty is PointerType
	if (!(IsPtrType(ty) && IsFunctionType(ty->bty))) {
		printf("ty is ptr and base ty is not function\n");
	}
	ty = ty->bty; // ty is FunctionType

	tail = (AstNode *)&expr->kids[1];
	arg = expr->kids[1];
	argNo = 1;
	argFull = 0;
	while (arg != NULL && !argFull)
	{
		*tail = (AstNode)CheckArgument((FunctionType)ty, arg, argNo, &argFull);
		tail = &(*tail)->next;
		arg = (AstExpression)arg->next;
		argNo++;
	}
	while (arg != NULL) {
		CheckExpression(expr);
		arg = (AstExpression)arg->next;
	}
	expr->ty = ty->bty;
	return expr;
}

//	a++,a--,++a,--a		------------->     a+= 1, a-=1
static AstExpression TransformIncrement(AstExpression expr)
{
	AstExpression casgn;
	union value val;
	
	val.i[1] = 0; val.i[0] = 1;
	CREATE_AST_NODE(casgn, Expression);
	casgn->op = (expr->op == OP_POSTINC || expr->op == OP_PREINC) ? OP_ADD_ASSIGN : OP_SUB_ASSIGN;
	casgn->kids[0] = expr->kids[0];
	casgn->kids[1] = Constant(T(INT), val);

	expr->kids[0] = CheckExpression(casgn);
	expr->ty = expr->kids[0]->ty;
	return expr;
}
static AstExpression CheckMemberAccess(AstExpression expr)
{
	Type ty;
	Field fld;
	expr->kids[0] = CheckExpression(expr->kids[0]);
	if (expr->op == OP_MEMBER) {
		expr->kids[0] = Adjust(expr->kids[0], 0);
		ty = expr->kids[0]->ty;
	} else {
		expr->kids[0] = Adjust(expr->kids[0], 1);
		ty = expr->kids[0]->ty;
		ty = ty->bty;
	}
	fld = LookupField(ty, (char*)expr->val.p);
	if (fld == NULL) {
		expr->ty = T(INT);
		return expr;
	}
	expr->ty = fld->ty;
	expr->val.p = fld;
	return expr;
}
static AstExpression ScalePointerOffset(AstExpression offset, int scale)
{
	AstExpression expr;
	union value val;
	CREATE_AST_NODE(expr, Expression);
	expr->ty = offset->ty;
	expr->op = OP_MUL;
	expr->kids[0] = offset;
	val.i[1] = 0;
	val.i[0] = scale;
	expr->kids[1] = Constant(offset->ty, val);
	return FoldConstant(expr);
}
static AstExpression CheckPostfixExpression(AstExpression expr)
{
	switch(expr->op) {
		case OP_INDEX:
			expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
			expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);
			if (IsObjectPtr(expr->kids[0]->ty) && IsIntegType(expr->kids[1]->ty)) {
				expr->ty = expr->kids[0]->ty->bty;
				// printf("cpe:%d\n", expr->ty->size);
				expr->kids[1] = ScalePointerOffset(expr->kids[1], expr->ty->size);	
			}
			return expr;

		case OP_CALL:
			return CheckFunctionCall(expr);
		case OP_POSTDEC:
		case OP_POSTINC:
			return TransformIncrement(expr);
		case OP_PTR_MEMBER:
		case OP_MEMBER:
			return CheckMemberAccess(expr);

		default:
			//assert(0);
			;
	}
}
static AstExpression CheckTypeCast(AstExpression expr)
{
	Type ty;
	ty = CheckTypeName((AstTypeName)expr->kids[0]);
	expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);
	return Cast(ty, expr->kids[1]);
}
static AstExpression CheckUnaryExpression(AstExpression expr)
{
	Type ty;
	
	switch(expr->op)
	{
	case OP_PREDEC:
	case OP_PREINC:
		return TransformIncrement(expr);

	case OP_ADDRESS: // &a
		expr->kids[0] = CheckExpression(expr->kids[0]);
		ty = expr->kids[0]->ty;
		/*
			int a[3];
			&a[3];  ---------->   pointer arithmetic operation
			Pointer(INT)    +  3
		*/
		if (expr->kids[0]->op == OP_INDEX) {
			expr->kids[0]->op = OP_ADD;
			expr->kids[0]->ty = PointerTo(ty);
			return expr->kids[0];
		}
		expr->ty = PointerTo(ty);
		return expr;
	case OP_DEREF: // *a
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		ty = expr->kids[0]->ty;
		if (IsPtrType(ty)) {
			expr->ty = ty->bty;
			if (IsFunctionType(expr->ty)) {
				return expr->kids[0];
			}
			return expr;
		}

	case OP_NOT: // !a
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		if (IsScalarType(expr->kids[0]->ty)) {
			expr->ty = T(INT);
			return FoldConstant(expr);
		}
		break;
	case OP_COMP:
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		if (IsIntegType(expr->kids[0]->ty)) {
			expr->ty = expr->kids[0]->ty;
			return FoldConstant(expr);
		}
	case OP_POS:
	case OP_NEG:
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		if (IsIntegType(expr->kids[0]->ty)) {
			expr->ty = expr->kids[0]->ty;
			return expr->op == OP_POS ? expr->kids[0] : FoldConstant(expr);
		}
	case OP_SIZEOF:
		if (expr->kids[0]->kind == NK_Expression) {
			expr->kids[0] = CheckExpression(expr->kids[0]);
			ty = expr->kids[0]->ty;
		}
		expr->ty = T(INT);
		expr->op = OP_CONST;
		expr->val.i[0] = ty->size;
		return expr;

	case OP_CAST: // (int)a
		return CheckTypeCast(expr);
	default:
		break;
	}
}
/**
 Syntax 
		   AND-expression:
				   equality-expression
				   AND-expression &  equality-expression 
				   
          exclusive-OR-expression:
                  AND-expression
                  exclusive-OR-expression ^  AND-expression		

          inclusive-OR-expression:
                  exclusive-OR-expression
                  inclusive-OR-expression |  exclusive-OR-expression                  
 Constraints 
	Each of the operands shall have integral type.	
 */
static AstExpression CheckBitwiseOP(AstExpression expr)
{
	if (BothIntegType(expr->kids[0]->ty, expr->kids[1]->ty))
	{
		PERFORM_ARITH_CONVERSION(expr);
		return FoldConstant(expr);
	}

	// REPORT_OP_ERROR;
}
/**
 Syntax
 
		   equality-expression:
				   relational-expression
				   equality-expression ==  relational-expression
				   equality-expression !=  relational-expression 

 */
/**
 Syntax 
		   shift-expression:
				   additive-expression
				   shift-expression <<	additive-expression
				   shift-expression >>	additive-expression
 
 Constraints
 
	Each of the operands shall have integral type.
	The integral promotions are performed on each of the operands.  The
type of the result is that of the promoted left operand.
 */
static AstExpression CheckShiftOP(AstExpression expr)
{
	if (BothIntegType(expr->kids[0]->ty, expr->kids[1]->ty))
	{
	// 	expr->kids[0] = DoIntegerPromotion(expr->kids[0]);
	// 	expr->kids[1] = DoIntegerPromotion(expr->kids[1]);
		expr->ty = expr->kids[0]->ty;

		return FoldConstant(expr);
	}

	// REPORT_OP_ERROR;
}
/**
 Syntax
		   multiplicative-expression:
				   cast-expression
				   multiplicative-expression *	cast-expression
				   multiplicative-expression /	cast-expression
				   multiplicative-expression %	cast-expression 
 Constraints
 
	Each of the operands shall have arithmetic type.  The operands of
 the % operator shall have integral type.

 */
static AstExpression CheckMultiplicativeOP(AstExpression expr)
{
	PERFORM_ARITH_CONVERSION(expr);
	return expr;
// 	if (expr->op != OP_MOD && BothArithType(expr->kids[0]->ty, expr->kids[1]->ty))
// 		goto ok;

// 	if (expr->op == OP_MOD && BothIntegType(expr->kids[0]->ty, expr->kids[1]->ty))
// 		goto ok;

// 	REPORT_OP_ERROR;

// ok:
// 	PERFORM_ARITH_CONVERSION(expr);
// 	return FoldConstant(expr);
}
static AstExpression CheckAddOP(AstExpression expr)
{
	Type ty1, ty2;
	if (expr->kids[0]->op == OP_CONST) {
		SWAP_KIDS(expr);
	}
	ty1 = expr->kids[0]->ty;
	ty2 = expr->kids[1]->ty;
	if (BothArithType(ty1, ty2)) {
		PERFORM_ARITH_CONVERSION(expr);
		return FoldConstant(expr);
	}
	REPORT_OP_ERROR;
}
static AstExpression CheckSubOP(AstExpression expr)
{
	PERFORM_ARITH_CONVERSION(expr);
	return expr;
}
static AstExpression CheckLogicalOP(AstExpression expr)
{
	expr->ty = T(INT);
	return expr;
}
static AstExpression CheckEqualityOP(AstExpression expr)
{
	expr->ty = T(INT);
	return expr;
}
static AstExpression CheckRelationalOP(AstExpression expr)
{
	Type ty1, ty2;
	expr->ty = T(INT);
	ty1 = expr->kids[0]->ty;
	ty2 = expr->kids[1]->ty;
	if (BothIntegType(ty1, ty2)) {
		PERFORM_ARITH_CONVERSION(expr);
		expr->ty = T(INT);
		return FoldConstant(expr);
	}
	return expr;
}
static AstExpression (* BinaryOPCheckers[])(AstExpression) = 
{
	CheckLogicalOP, // ||
	CheckLogicalOP, // &&
	CheckBitwiseOP,	// |
	CheckBitwiseOP, // ^
	CheckBitwiseOP,	// &
	CheckEqualityOP, // ==
	CheckEqualityOP, // !=
	CheckRelationalOP, // >
	CheckRelationalOP, // <
	CheckRelationalOP, // >=
	CheckRelationalOP, // <=
	CheckShiftOP,		//	<<
	CheckShiftOP,		// >>
	CheckAddOP,			// +
	CheckSubOP,			// -
	CheckMultiplicativeOP,	// *
	CheckMultiplicativeOP,	//  /
	CheckMultiplicativeOP	// %	
};
static AstExpression CheckBinaryExpression(AstExpression expr)
{
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
	expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);

	return (* BinaryOPCheckers[expr->op - OP_OR])(expr);
}
/**
 Syntax
 
		   conditional-expression:
				   logical-OR-expression
				   logical-OR-expression ?	expression :  conditional-expression
 OPINFO(OP_QUESTION,	  3,	"?",	  Conditional,	  NOP)
 */
static AstExpression CheckConditionalExpression(AstExpression expr)
{
	Type ty1, ty2;
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
	expr->kids[1]->kids[0] = Adjust(CheckExpression(expr->kids[1]->kids[0]), 1);
	expr->kids[1]->kids[1] = Adjust(CheckExpression(expr->kids[1]->kids[1]), 1);
	ty1 = expr->kids[1]->kids[0]->ty;
	ty2 = expr->kids[1]->kids[1]->ty;
	if (BothArithType(ty1, ty2)) {
		expr->ty = CommonRealType(ty1, ty2);
		expr->kids[1]->kids[0] = Cast(expr->ty, expr->kids[1]->kids[0]);
		expr->kids[1]->kids[1] = Cast(expr->ty, expr->kids[1]->kids[1]);
		return expr;
	}
}
/**
 Syntax
 
		   assignment-expression:
				   conditional-expression
				   unary-expression assignment-operator assignment-expression
 
		   assignment-operator: one of
				   =  *=  /=  %=  +=  -=  <<=  >>=	&=	^=	|=s

OPINFO(OP_ASSIGN,        2,    "=",      Assignment,     NOP)
OPINFO(OP_BITOR_ASSIGN,  2,    "|=",     Assignment,     NOP)
OPINFO(OP_BITXOR_ASSIGN, 2,    "^=",     Assignment,     NOP)
OPINFO(OP_BITAND_ASSIGN, 2,    "&=",     Assignment,     NOP)
OPINFO(OP_LSHIFT_ASSIGN, 2,    "<<=",    Assignment,     NOP)
OPINFO(OP_RSHIFT_ASSIGN, 2,    ">>=",    Assignment,     NOP)
OPINFO(OP_ADD_ASSIGN,    2,    "+=",     Assignment,     NOP)
OPINFO(OP_SUB_ASSIGN,    2,    "-=",     Assignment,     NOP)
OPINFO(OP_MUL_ASSIGN,    2,    "*=",     Assignment,     NOP)
OPINFO(OP_DIV_ASSIGN,    2,    "/=",     Assignment,     NOP)
OPINFO(OP_MOD_ASSIGN,    2,    "%=",     Assignment,     NOP)				   
*/
static AstExpression CheckAssignmentExpression(AstExpression expr)
{
	int ops[] = 
	{ 
		OP_BITOR, OP_BITXOR, OP_BITAND, OP_LSHIFT, OP_RSHIFT, 
		OP_ADD,	  OP_SUB,    OP_MUL,    OP_DIV,    OP_MOD 
	};	
	Type ty;
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 0);
	expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);

	if (!CanModify(expr->kids[0])) {
		Error(NULL, "The Left operand cannot be modified!");
	}
	if (expr->op != OP_ASSIGN) {
		AstExpression lopr;
		CREATE_AST_NODE(lopr, Expression);
		lopr->op = ops[expr->op - OP_BITOR_ASSIGN];
		lopr->kids[0] = expr->kids[0];
		lopr->kids[1] = expr->kids[1];
		expr->kids[1] = (*BinaryOPCheckers[lopr->op - OP_OR])(lopr);

	}
	// we have use CanModify() to test whether left operand is modifiable.
	ty = expr->kids[0]->ty;
	expr->kids[1] = Cast(ty, expr->kids[1]);	
	expr->ty = ty;	
	return expr;
}

static AstExpression CheckCommaExpression(AstExpression expr)
{
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
	expr->kids[0] = Adjust(CheckExpression(expr->kids[1]), 1);

	expr->ty = expr->kids[1]->ty;
	return expr;
}

static AstExpression CheckErrorExpression(AstExpression expr)
{
	return NULL;
}

static AstExpression (*ExprCheckers[])(AstExpression) = 
{
#define OPINFO(op, prec, name, func, opcode) Check##func##Expression,
#include "opinfo.h"
#undef OPINFO
};

AstExpression CheckExpression(AstExpression expr)
{
	return (*ExprCheckers[expr->op])(expr);
}

AstExpression CheckConstantExpression(AstExpression expr)
{
	expr = CheckExpression(expr);
	if (!(expr->op == OP_CONST && IsIntegType(expr->ty))) {
		return NULL;
	}
	return expr;
}
