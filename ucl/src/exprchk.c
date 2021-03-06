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

	return (expr->lvalue && !(expr->ty->qual & CONST) &&
		(IsRecordType(expr->ty) ? !((RecordType)expr->ty)->hasConstFld : 1));
}
 AstExpression DoIntegerPromotion(AstExpression expr)
 {
     return expr->ty->categ < INT ? Cast(T(INT), expr) : expr;
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
	if (dcode == V)
		return CastExpression(ty, expr);
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
	if (scode < I4) {
		expr = CastExpression(T(INT), expr);
		scode = I4;
	}
	if (scode != dcode) {
		if (dcode < I4) {
			expr = CastExpression(T(INT), expr);
		}
		expr = CastExpression(ty, expr);
	}
	return expr;
}
static int IsNullConstant(AstExpression expr)
{
	return expr->op == OP_CONST && expr->val.i[0] == 0;
}
int CanAssign(Type lty, AstExpression expr)
{
	Type rty = expr->ty;
	lty = Unqual(lty);
	rty = Unqual(rty);
	if (lty == rty || BothArithType(lty, rty)) {
		return 1;
	}
	// T1 * and T2 *
	if (IsCompatiblePtr(lty, rty) && ((lty->bty->qual & rty->bty->qual) == rty->bty->qual)) 
		return 1;
	if ((NotFunctionPtr(lty) && IsVoidPtr(rty) || NotFunctionPtr(rty) && IsVoidPtr(lty)) &&
		((lty->bty->qual & rty->bty->qual) == rty->bty->qual))
		return 1;
	if (IsPtrType(lty) && IsNullConstant(expr))
		return 1;
	
	if (IsPtrType(lty) && IsPtrType(rty)) {
		return 1;
	}
	if ((IsPtrType(lty) && IsIntegType(rty) || IsPtrType(rty) && IsIntegType(lty))&&
         (lty->size == rty->size))
     {
         Warning(NULL, "conversion between pointer and integer without a cast");
         return 1;
     }
	return 0;
}
// ptr2 - ptr1 => (ptr2 - ptr1) / sizeof(*ptr2);
static AstExpression PointerDifference(AstExpression diff, int size)
{
	AstExpression expr;
	union value val;
	CREATE_AST_NODE(expr, Expression);

	expr->ty = diff->ty;
	expr->op = OP_DIV;
	expr->kids[0] = diff;
	val.i[1] = 0;
	val.i[0] = size;
	expr->kids[1] = Constant(diff->ty, val);
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
		expr->lvalue = 1;
		return expr;
	}	
	p = LookupID((char*)expr->val.p);
	if (p == NULL) {
		Error(NULL, "Undeclared identified: %s", expr->val.p);
		p = AddVariable((char*)expr->val.p, T(INT), Level == 0 ? 0 : TK_AUTO);
		expr->ty = T(INT);
		expr->lvalue = 1;
	} else if (p->kind == SK_TypedefName) {
		Error(NULL, "Typedef name cannot be used as variable");
		expr->ty = T(INT);
	} else if (p->kind == SK_EnumConstant) {
		expr->op = OP_CONST;
		expr->ty = T(INT);
		expr->val = p->val;
	} else {
		expr->ty = p->ty;
		expr->val.p = p;
		// an ID is a lvalue, while a function designator not
		expr->lvalue = expr->ty->categ != FUNCTION;
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
		expr->lvalue = 0;
	}
	if (expr->ty->categ == FUNCTION) {
		expr->ty = PointerTo(expr->ty);
		expr->isfunc = 1;
	} else if (expr->ty->categ == ARRAY) {
		expr->ty = PointerTo(Qualify(qual, expr->ty->bty));
		expr->isarray = 1;
		expr->lvalue = 0;
	}
	return expr;
}

static AstExpression CheckArgument(FunctionType fty, AstExpression arg, int argNo, int *argFull)
{
	Parameter param;

	int parLen = LEN(fty->sig->params);
	arg = Adjust(CheckExpression(arg), 1);
	
	// f(void) 
	if (parLen == 0)
	{
		*argFull = 1;
		return arg;
	}
	// f(int, int, int)  and check the last one parameter			
	if (argNo == parLen && ! fty->sig->hasEllipsis)
		*argFull = 1;

	if (argNo <= parLen) {
		param = (Parameter)GET_ITEM(fty->sig->params, argNo - 1);
		// 
		if (!CanAssign(param->ty, arg))
			goto err;
		if (param->ty->categ < INT)
			arg = Cast(T(INT), arg);
		else
			arg = Cast(param->ty, arg);
		return arg;
	} else {
		// variable arguments
		return PromoteArgument(arg);
	}
err:
	Error(NULL, "Incompatible argument");
	return arg;
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Type ty;
	AstNode *tail;
	int argNo, argFull;

	if (expr->kids[0]->op == OP_ID && LookupID((char*)expr->kids[0]->val.p) == NULL) {
		Error(NULL, "Function %s not declared", expr->kids[0]->val.p);
		expr->kids[0]->ty = DefaultFunctionType;
		expr->kids[0]->val.p = AddFunction((char*)expr->kids[0]->val.p, DefaultFunctionType, TK_EXTERN);
	} else {
		expr->kids[0] = CheckExpression(expr->kids[0]);
	}
	expr->kids[0] = Adjust(expr->kids[0], 1);
	ty = expr->kids[0]->ty; // ty is PointerType
	if (!(IsPtrType(ty) && IsFunctionType(ty->bty))) {
		Error(NULL, "ty is ptr and base ty is not function\n");
		ty = DefaultFunctionType;
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
	*tail = NULL;
	while (arg != NULL) {
		CheckExpression(expr);
		arg = (AstExpression)arg->next;
	}
	argNo--;
	if (argNo > LEN(((FunctionType)ty)->sig->params)) {
		if (!((FunctionType)ty)->sig->hasEllipsis) {
			Error(NULL, "Too many arguments");
		}
	} else if (argNo < LEN(((FunctionType)ty)->sig->params)) {
		if (!((FunctionType)ty)->sig->hasEllipsis) {
			Error(NULL, "Too few arguments");
		}
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
			/**
             typedef struct{
                 int a;
                 int b;
             }Data;
             Data dt;
             dt.a = 3;           // legal        lvalue is 1
             GetData().a = 3;    // illegal      lvalue is 0
          */
		expr->kids[0] = Adjust(expr->kids[0], 0);
		ty = expr->kids[0]->ty;
		if (!IsRecordType(ty)) {
			REPORT_OP_ERROR;
		}
		expr->lvalue = expr->kids[0]->lvalue;
	} else {
		/**
         For example:
             Type ty;
             ((FunctionType) ty)->sig;           // the whole expression, lvalue is 1
             ((FunctionType) ty) is considered as a right value. //lvalue is 0
          */
		expr->kids[0] = Adjust(expr->kids[0], 1);
		ty = expr->kids[0]->ty;
		if (!IsPtrType(ty) && !IsRecordType(ty)) {
			REPORT_OP_ERROR;
		}
		ty = ty->bty;
		expr->lvalue = 1;
	}
	fld = LookupField(ty, (char*)expr->val.p);
	if (fld == NULL) {
		Error(NULL, "struct member %s doesn't exist", expr->val.p);
		expr->ty = T(INT);
		return expr;
	}
	expr->ty = Qualify(ty->qual, fld->ty);
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
			// do not support 1[arr] = 2;
			if (IsObjectPtr(expr->kids[0]->ty) && IsIntegType(expr->kids[1]->ty)) {
				expr->ty = expr->kids[0]->ty->bty;
				/*
					int arr[2][1];
					arr[1]  is considered a left value here(not accurate)                                
					arr[1] is an array, not a left value.                                                
					Later, jump out recursive, upper Adjust(...) will set lvalue to 0. 
					And arr[1][1] can be a left value
				*/
				expr->lvalue = 1;
				expr->kids[1] = DoIntegerPromotion(expr->kids[1]); // 把short,char提升至int类型
				expr->kids[1] = ScalePointerOffset(expr->kids[1], expr->ty->size);	
				if (!expr->kids[0]->isarray && expr->ty->categ != ARRAY) {
					AstExpression deref, addExpr;
					CREATE_AST_NODE(deref, Expression);
					CREATE_AST_NODE(addExpr, Expression);
					deref->op = OP_DEREF;
					deref->ty = expr->kids[0]->ty->bty;
					deref->kids[0] = addExpr;

					addExpr->op = OP_ADD;
					addExpr->ty = expr->kids[0]->ty;
					addExpr->kids[0] = expr->kids[0];
					addExpr->kids[1] = expr->kids[1];
					deref->lvalue = 1;
					return deref;
				}
				return expr;
			}
			REPORT_OP_ERROR;

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
	REPORT_OP_ERROR;
}
static AstExpression CheckTypeCast(AstExpression expr)
{
	Type ty;
	ty = CheckTypeName((AstTypeName)expr->kids[0]);
	expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);
	if (! (BothScalarType(ty, expr->kids[1]->ty) || ty->categ == VOID)) {
		Error(NULL, "Illegal type cast");
		return expr->kids[1];
	}
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
		if (expr->kids[0]->op == OP_DEREF) {
			// &(*ptr) -> ptr
			expr->kids[0]->kids[0]->lvalue = 0;
			return expr->kids[0]->kids[0];
		}
		/*
			int a[3];
			&a[3];  ---------->   pointer arithmetic operation
			Pointer(INT)    +  3
		*/
		else if (expr->kids[0]->op == OP_INDEX) {
			expr->kids[0]->op = OP_ADD;
			expr->kids[0]->ty = PointerTo(ty);
			expr->kids[0]->lvalue = 0;
			return expr->kids[0];
		} else if (IsFunctionType(ty) || expr->kids[0]->lvalue) {
			// &func
			expr->lvalue = 0;
			expr->ty = PointerTo(ty);
			return expr;
		}
		break;

	case OP_DEREF: // *a
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		ty = expr->kids[0]->ty;
		if (expr->kids[0]->op == OP_ADDRESS) {
			// *&a;
			expr->kids[0]->kids[0]->ty = ty->bty;
			return expr->kids[0]->kids[0];
		} else if (expr->kids[0]->op == OP_ADD && (ty->bty->categ == ARRAY || expr->kids[0]->kids[0]->isarray)) {
			// *(arr+3) -> arr[3]
			expr->kids[0]->op = OP_INDEX;
			expr->kids[0]->ty = ty->bty;
			expr->kids[0]->lvalue = 1;
			return expr->kids[0];
		}
		if (IsPtrType(ty)) {
			expr->ty = ty->bty;
			// void f(void) {}
			if (IsFunctionType(expr->ty)) {
				// (*f)() -> f();
				return expr->kids[0];
			} 
			// *ptr -> ptr[0]
			if (expr->ty->categ == ARRAY || expr->kids[0]->isarray) {
				union value val;
				val.i[0] = val.i[1] = 0;
				expr->kids[1] = Constant(T(INT), val);
				expr->op = OP_INDEX;
			}
			expr->lvalue = 1;
			return expr;
		}
		break;

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
			expr->kids[0] = DoIntegerPromotion(expr->kids[0]);
			expr->ty = expr->kids[0]->ty;
			return FoldConstant(expr);
		}
	case OP_POS:
	case OP_NEG:
		expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
		if (IsIntegType(expr->kids[0]->ty)) {
			expr->kids[0] = DoIntegerPromotion(expr->kids[0]);
			expr->ty = expr->kids[0]->ty;
			return expr->op == OP_POS ? expr->kids[0] : FoldConstant(expr);
		}
	case OP_SIZEOF:
		if (expr->kids[0]->kind == NK_Expression) {
			expr->kids[0] = CheckExpression(expr->kids[0]);
			ty = expr->kids[0]->ty;
		} else {
			ty = CheckTypeName((AstTypeName)expr->kids[0]);
		}
		if (IsFunctionType(ty)) 
			goto err;
		expr->ty = T(UINT);
		expr->op = OP_CONST;
		expr->val.i[0] = ty->size;
		return expr;

	case OP_CAST: // (int)a
		return CheckTypeCast(expr);
	default:
		break;
	}
err:
	REPORT_OP_ERROR;
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

	REPORT_OP_ERROR;
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
		expr->kids[0] = DoIntegerPromotion(expr->kids[0]);
		expr->kids[1] = DoIntegerPromotion(expr->kids[1]);
		expr->ty = expr->kids[0]->ty;

		return FoldConstant(expr);
	}

	REPORT_OP_ERROR;
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
	if (expr->op != OP_MOD && BothArithType(expr->kids[0]->ty, expr->kids[1]->ty))
		goto ok;

	if (expr->op == OP_MOD && BothIntegType(expr->kids[0]->ty, expr->kids[1]->ty))
		goto ok;

	REPORT_OP_ERROR;

ok:
	PERFORM_ARITH_CONVERSION(expr);
	return FoldConstant(expr);
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
	if (IsObjectPtr(ty2) && IsIntegType(ty1)) {
		SWAP_KIDS(expr);
		ty1 = expr->kids[0]->ty;
		goto left_ptr;
	}
	if (IsObjectPtr(ty1) && IsIntegType(ty2)) {
left_ptr:
		expr->kids[1] = DoIntegerPromotion(expr->kids[1]);
		expr->kids[1] = ScalePointerOffset(expr->kids[1], ty1->bty->size);
		expr->ty = ty1;
		return expr;
	}
	REPORT_OP_ERROR;
}
static AstExpression CheckSubOP(AstExpression expr)
{
	Type ty1, ty2;
	ty1 = expr->kids[0]->ty;
	ty2 = expr->kids[1]->ty;
	// 3 + 4;
	if (BothArithType(ty1, ty2)) {
		PERFORM_ARITH_CONVERSION(expr);
		return FoldConstant(expr);
	}
	// ptr - 3;
	if (IsObjectPtr(ty1) && IsIntegType(ty2)) {
		expr->kids[1] = DoIntegerPromotion(expr->kids[1]);
		expr->kids[1] = ScalePointerOffset(expr->kids[1], ty1->bty->size);
		expr->ty = ty1;
		return expr;
	}
	// int arr[10];
	// int len = &arr[9] - &arr[1];
	if (IsCompatiblePtr(ty1, ty2)) {
		expr->ty = T(INT);
		expr = PointerDifference(expr, ty1->bty->size);
		return expr;
	}
	REPORT_OP_ERROR;
}
static AstExpression CheckLogicalOP(AstExpression expr)
{
	if (BothScalarType(expr->kids[0]->ty, expr->kids[1]->ty)) { 
		expr->ty = T(INT);
		return FoldConstant(expr);
	}
	REPORT_OP_ERROR;
}
static AstExpression CheckEqualityOP(AstExpression expr)
{
	Type ty1, ty2;
	expr->ty = T(INT);
	ty1 = expr->kids[0]->ty;
	ty2 = expr->kids[1]->ty;
	if (BothArithType(ty1, ty2)) {
		PERFORM_ARITH_CONVERSION(expr);
		expr->ty = T(INT);
		return FoldConstant(expr);
	}
	if (IsCompatiblePtr(ty1, ty2) || 
		NotFunctionPtr(ty1) && IsVoidPtr(ty2) || 
		NotFunctionPtr(ty2) && IsVoidPtr(ty1) ||
		IsPtrType(ty1) && IsNullConstant(expr->kids[1]) ||
		IsPtrType(ty2) && IsNullConstant(expr->kids[0])) {
		return expr;
	}
	REPORT_OP_ERROR;
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
	int qual;
	Type ty1, ty2;
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
	if (! IsScalarType(expr->kids[0]->ty)) {
		Error(NULL, "The first expression shall be scalar type");
	}
	expr->kids[1]->kids[0] = Adjust(CheckExpression(expr->kids[1]->kids[0]), 1);
	expr->kids[1]->kids[1] = Adjust(CheckExpression(expr->kids[1]->kids[1]), 1);
	ty1 = expr->kids[1]->kids[0]->ty;
	ty2 = expr->kids[1]->kids[1]->ty;
	if (BothArithType(ty1, ty2)) {
		expr->ty = CommonRealType(ty1, ty2);
		expr->kids[1]->kids[0] = Cast(expr->ty, expr->kids[1]->kids[0]);
		expr->kids[1]->kids[1] = Cast(expr->ty, expr->kids[1]->kids[1]);
		return FoldConstant(expr);
	} else if (IsRecordType(ty1) && ty1 == ty2) {
		expr->ty = ty1;
	} else if (ty1->categ == VOID && ty2->categ == VOID) {
		expr->ty = T(VOID);
	} else if (IsCompatiblePtr(ty1, ty2)) {
		qual = ty1->bty->qual | ty2->bty->qual;
		expr->ty = PointerTo(Qualify(qual, CompositeType(Unqual(ty1->bty), Unqual(ty2->bty))));
	} else if (IsPtrType(ty1) && IsNullConstant(expr->kids[1]->kids[1])) {
		expr->ty = ty1;
	} else if (IsPtrType(ty2) && IsNullConstant(expr->kids[1]->kids[0])) {
		expr->ty = ty2;
	} else if (NotFunctionPtr(ty1) && IsVoidPtr(ty2) || NotFunctionPtr(ty2) && IsVoidPtr(ty1)) {
		qual = ty1->bty->qual | ty2->bty->qual;
		expr->ty = PointerTo(Qualify(qual, T(VOID)));	
	} else {
		Error(NULL, "invalid operand for ? operator");
		expr->ty = T(INT);
	}
	return expr;	
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
	// a += b -> a = a' + b
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
	if (!CanAssign(ty, expr->kids[1])) {
		Error(NULL, "Wrong assignment");
	} else {
		expr->kids[1] = Cast(ty, expr->kids[1]);	
	}
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
