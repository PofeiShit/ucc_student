#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "gen.h"

/**
 * Translates a primary expression.
 */
static Symbol TranslatePrimaryExpression(AstExpression expr)
{
	if (expr->op == OP_CONST)
		return AddConstant(expr->ty, expr->val);
	/// if the expression is adjusted from an array or a function,
	/// returns the address of the symbol for this identifier
	if (expr->op == OP_STR || expr->isarray || expr->isfunc) {
		//#if  1	// added
		//assert(expr->op != OP_STR);
		//#endif
		return AddressOf((Symbol)expr->val.p);
	}

	return (Symbol)expr->val.p;
}

/**
	 postfix-expression:
			 primary-expression
			 postfix-expression (  argument-expression-list<opt> ) 
			 
	expe->op	is 	OP_CALL
*/
static Symbol TranslateFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Symbol faddr, recv;
	ILArg ilarg;
	Vector args = CreateVector(4);
	/**	
		Here, we want to use function name f as function call?
		Function name can be used as function address,
			see TranslatePrimaryExpression(AstExpression expr).
	 */
	expr->kids[0]->isfunc = 0;
	faddr = TranslateExpression(expr->kids[0]);
	arg = expr->kids[1];
	
	while (arg)
	{
		ALLOC(ilarg);
		// After TranslateExpression(arg), sym->ty may be different from art->ty.
		// See AddressOf() and the following comments for detail.
		ilarg->sym = TranslateExpression(arg);
		ilarg->ty = arg->ty;
		INSERT_ITEM(args, ilarg);
		arg = (AstExpression)arg->next;
	}
	
	recv = NULL;
			
	if (expr->ty->categ != VOID)
	{
		recv = CreateTemp(expr->ty);		
	}
	GenerateFunctionCall(expr->ty, recv, faddr, args);
	return recv;
}
static Symbol TranslateIncrement(AstExpression expr)
{
	AstExpression casgn;
	Symbol p;
	casgn = expr->kids[0];
	p = TranslateExpression(casgn->kids[0]);
	if (expr->op == OP_POSTINC || expr->op == OP_POSTDEC) {
		Symbol ret;
		ret = p;
		if (p->kind != SK_Temp) {
			ret = CreateTemp(expr->ty);
			GenerateMove(expr->ty, ret, p);
		}
		TranslateExpression(casgn);
		return ret;
	}
	return TranslateExpression(casgn);
}

// voff: variable offset, coff: constant offset
static Symbol Offset(Type ty, Symbol addr, Symbol voff, int coff)
{
	// TranslateMemberAccess
	if (addr->kind == SK_Temp) {
		return CreateOffset(ty, addr, coff);
	}
	return Deref(ty, Simplify(T(POINTER), ADD, addr, IntConstant(coff))); 
}
static Symbol TranslateMemberAccess(AstExpression expr)
{
	AstExpression p;
	Field fld;
	Symbol addr, dst, tmp;
	p = expr;
	int coff = 0;
	if (p->op == OP_MEMBER) {
		while (p->op == OP_MEMBER) {
			fld = (Field)p->val.p;
			coff += fld->offset;
			p = p->kids[0];
		}
		tmp = TranslateExpression(p);
		// addr = AddressOf(tmp);
		dst = CreateOffset(expr->ty, tmp, coff);
	} else {
		fld = (Field)p->val.p;
		coff = fld->offset;
		addr = TranslateExpression(expr->kids[0]);
		if (expr->lvalue)
			return Simplify(T(POINTER), ADD, addr, IntConstant(coff));
		dst = Deref(expr->ty, Simplify(T(POINTER), ADD, addr, IntConstant(coff)));
		return dst;
	}
	// dst = Offset(expr->ty, addr, NULL, coff);
	return dst;
}
static Symbol TranslateArrayIndex(AstExpression expr)
{
	AstExpression p;
	Symbol addr, dst, voff = NULL, tmp;
	int coff = 0;
	p = expr;
	do 
	{
		if (p->kids[1]->op == OP_CONST) {
			coff += p->kids[1]->val.i[0];	
		} else if (voff == NULL) {
			voff = TranslateExpression(p->kids[1]);
		} else {
			voff = Simplify(voff->ty, ADD, voff, TranslateExpression(p->kids[1]));
		}
		p = p->kids[0];

	} while (p->op == OP_INDEX);
	if (!p->isarray) {
		/*
			int arr[3][4];
			typedef int (*ArrPtr)[4];
			ArrPtr ptr = &arr[0];
			void main()
			{
				ptr[1][2] = 1; // get here
			}
		*/
		addr = Simplify(T(POINTER), ADD, (Symbol)p->val.p, IntConstant(coff));
		if (expr->lvalue) {
			expr->op = OP_DEREF;
			return addr;
		}
		return Deref(expr->ty, addr);
	}
	// dst = Offset(expr->ty, addr, voff, coff);
	if (voff != NULL) {
		addr = TranslateExpression(p);
		if (p->op == OP_PTR_MEMBER || p->op == OP_MEMBER)
			// dt->num[i] = 50
			addr = AddressOf(addr);
		voff = Simplify(T(POINTER), ADD, voff, IntConstant(coff));
		addr = Simplify(T(POINTER), ADD, addr, voff);
		if (expr->lvalue) {
			expr->op = OP_DEREF;
			return addr;
		}
		dst = Deref(expr->ty, addr);
	} else {
		if (p->op == OP_MEMBER || p->op == OP_PTR_MEMBER) {
			// p->name[3] = 50; p.name[3] = 50;
			addr = TranslateExpression(p);
		} else {
			addr = (Symbol)p->val.p;
		}
		dst = CreateOffset(expr->ty, addr, coff);
	}
	return expr->isarray ? AddressOf(dst) : dst;
}

static Symbol TranslatePostfixExpression(AstExpression expr)
{
	switch (expr->op)
	{
	case OP_INDEX:
		return TranslateArrayIndex(expr);
	case OP_CALL:
		return TranslateFunctionCall(expr);
	case OP_POSTDEC:
	case OP_POSTINC:
		return TranslateIncrement(expr);
	case OP_MEMBER:
	case OP_PTR_MEMBER:
		return TranslateMemberAccess(expr);
	default:
		//assert(0);
		;
		return NULL;
	}
}

static Symbol TranslateBranchExpression(AstExpression expr)
{
	BBlock nextBB, trueBB, falseBB;
	Symbol t;
	t = CreateTemp(expr->ty);
	nextBB = CreateBBlock();
	trueBB = CreateBBlock();
	falseBB = CreateBBlock();

	TranslateBranch(expr, trueBB, falseBB);
	StartBBlock(falseBB);
	GenerateMove(expr->ty, t, IntConstant(0));
	// goto BB1
	GenerateJump(nextBB);
	StartBBlock(trueBB);
	GenerateMove(expr->ty, t, IntConstant(1));
	StartBBlock(nextBB);
	return t;
}
static Symbol TranslateCast(Type ty, Type sty, Symbol src)
{
	Symbol dst;
	int scode, dcode, opcode;
	dcode = TypeCode(ty);
	scode = TypeCode(sty);
	if (scode == dcode) {
		return src;
	}
	if (dcode == V)
		return NULL;
	switch(scode) 
	{
	case I1:
		opcode = EXTI1; break;
	case I2:
		opcode = EXTI2; break;
	case U1:
		opcode = EXTU1; break;
	case U2:
		opcode = EXTU2; break;
	case I4:
		if (dcode <= U1) 
			opcode = TRUI1;
		else if (dcode <= U2)
			opcode = TRUI2;
		else {
			Symbol temp = CreateTemp(ty);
			GenerateMove(ty, temp, src);
			return temp;
		}
		break;

	case U4:
		{
			Symbol temp = CreateTemp(ty);
			GenerateMove(ty, temp, src);
			return temp;
		}
		break;
	default:
		;
	}
	dst = CreateTemp(ty);
	GenerateAssign(ty, dst, opcode, src, NULL);
	return dst;
}
static Symbol TranslateUnaryExpression(AstExpression expr)
{
	Symbol src;
	if (expr->op == OP_NOT) {
		return TranslateBranchExpression(expr);
	}
	if (expr->op == OP_PREINC || expr->op == OP_PREDEC) {
		return TranslateIncrement(expr);
	}
	src = TranslateExpression(expr->kids[0]);
	switch(expr->op) 
	{
	case OP_CAST:
		return TranslateCast(expr->ty, expr->kids[0]->ty, src);
	case OP_ADDRESS:
		return AddressOf(src);
	case OP_DEREF: // *a
		/*
		case *a = 1 is a left value, Deref is done by TranslateAssignment without duplicate.
		case c = *a is Deref below;
		*/
		return expr->lvalue ? src : Deref(expr->ty, src);
	case OP_NEG:
	case OP_COMP:
		return Simplify(expr->ty, OPMap[expr->op], src, NULL);
	default:
		return NULL;
	}
}
static Symbol TranslateBinaryExpression(AstExpression expr)
{
	Symbol src1, src2;
	if (expr->op == OP_OR || expr->op == OP_AND || (expr->op >= OP_EQUAL && expr->op <= OP_LESS_EQ)) {
		return TranslateBranchExpression(expr);
	}
	src1 = TranslateExpression(expr->kids[0]);
	src2 = TranslateExpression(expr->kids[1]);
	return Simplify(expr->ty, OPMap[expr->op], src1, src2);
}
static Symbol TranslateConditionalExpression(AstExpression expr)
{
	Symbol t, t1, t2;
	BBlock trueBB, falseBB, nextBB;
	t = NULL;
	if (expr->ty->categ != VOID) {
		t = CreateTemp(expr->ty);
	}
	trueBB = CreateBBlock();
	falseBB = CreateBBlock();
	nextBB = CreateBBlock();
	TranslateBranch(Not(expr->kids[0]), falseBB, trueBB);

	StartBBlock(trueBB);
	t1 = TranslateExpression(expr->kids[1]->kids[0]);
	if (t1 != NULL)
		GenerateMove(expr->ty, t, t1);
	GenerateJump(nextBB);

	StartBBlock(falseBB);
	t2 = TranslateExpression(expr->kids[1]->kids[1]);
	if (t2 != NULL)
		GenerateMove(expr->ty, t, t2);

	StartBBlock(nextBB);
	return t;
}
static Symbol TranslateAssignmentExpression(AstExpression expr)
{
	Symbol dst, src;
	dst = TranslateExpression(expr->kids[0]);
	if (expr->op != OP_ASSIGN) {
		// *f() += 3
		if (expr->kids[0]->op == OP_DEREF) {
			expr->kids[0]->val.p = Deref(expr->kids[0]->ty, dst);
		}
		expr->kids[0]->op = OP_ID;
	}
	src = TranslateExpression(expr->kids[1]);
	if (expr->kids[0]->op == OP_PTR_MEMBER || expr->kids[0]->op == OP_DEREF || dst->kind == SK_Temp) {
		GenerateIndirectMove(expr->ty, dst, src); // dst is base address
		// dst = Deref(expr->ty, dst);
	} else {
		GenerateMove(expr->ty, dst, src);
	}
	return dst;
}
static Symbol TranslateCommaExpression(AstExpression expr)
{
	TranslateExpression(expr->kids[0]);
	return TranslateExpression(expr->kids[1]);
}

static Symbol TranslateErrorExpression(AstExpression expr)
{
	return NULL;
}
static Symbol (* ExprTrans[])(AstExpression) = 
{
#define OPINFO(op, prec, name, func, opcode) Translate##func##Expression,
#include "opinfo.h"
#undef OPINFO
};
AstExpression Not(AstExpression expr)
{
	static int rops[] = {OP_UNEQUAL, OP_EQUAL, OP_LESS_EQ, OP_GREAT_EQ, OP_LESS, OP_GREAT};
	AstExpression t;
	switch(expr->op)
	{
	case OP_AND:
		expr->op = OP_OR;
		expr->kids[0] = Not(expr->kids[0]);
		expr->kids[1] = Not(expr->kids[1]);
		return expr;
	case OP_OR:
		expr->op = OP_AND;
		expr->kids[0] = Not(expr->kids[0]);
		expr->kids[1] = Not(expr->kids[1]);
		return expr;
	case OP_EQUAL:
	case OP_UNEQUAL:
	case OP_GREAT:
	case OP_LESS:
	case OP_GREAT_EQ:
	case OP_LESS_EQ:
		expr->op = rops[expr->op - OP_EQUAL];
		return expr;
	
	case OP_NOT:
		return expr->kids[0];

	default:
		CREATE_AST_NODE(t, Expression);
		t->ty = T(INT);
		t->op = OP_NOT;
		t->kids[0] = expr;
		return t;
	}
}
void TranslateBranch(AstExpression expr, BBlock trueBB, BBlock falseBB)
{
	BBlock rtestBB;
	Symbol src1, src2;
	Type ty;
	switch(expr->op) {
	case OP_CONST:
		if (! (expr->val.i[0] == 0 && expr->val.i[1] == 0))
		{			
			GenerateJump(trueBB);
		}
		break;
	case OP_AND:
		rtestBB = CreateBBlock();
		TranslateBranch(Not(expr->kids[0]), falseBB, rtestBB);
		StartBBlock(rtestBB);
		TranslateBranch(expr->kids[1], trueBB, falseBB);
		break;
	case OP_OR:
		rtestBB = CreateBBlock();
		TranslateBranch(expr->kids[0], trueBB, rtestBB);
		StartBBlock(rtestBB);
		TranslateBranch(expr->kids[1], trueBB, falseBB);
		break;
	case OP_GREAT:
	case OP_LESS:
	case OP_GREAT_EQ:
	case OP_LESS_EQ:
	case OP_UNEQUAL:
	case OP_EQUAL:
		src1 = TranslateExpression(expr->kids[0]);
		src2 = TranslateExpression(expr->kids[1]);
		GenerateBranch(expr->kids[0]->ty, trueBB, OPMap[expr->op], src1, src2);
		break;

	case OP_NOT:
		{
			// (!!!!!a)
			int count = 1;
			AstExpression parent = expr;
			AstExpression child = expr->kids[0];
			while (child->op == OP_NOT) {
				parent = child;
				child = child->kids[0];
				count++;
			}
			src1 = TranslateExpression(parent->kids[0]);
			ty = parent->kids[0]->ty;
			if (ty->categ < INT) {
				src1 = TranslateCast(T(INT), ty, src1);
				ty = T(INT);
			}
			if (count % 2 == 1) 
				GenerateBranch(ty, trueBB, JZ, src1, NULL);
			else 
				GenerateBranch(ty, trueBB, JNZ, src1, NULL);
		}
		break;
	default:
		// a + b;
		src1 = TranslateExpression(expr);
		if (src1->kind == SK_Constant) {
			if (! (src1->val.i[0] == 0 && src1->val.i[1] == 0)) {
				GenerateJump(trueBB);
			}
		} else {
			ty = expr->ty;
			if (ty->categ < INT) {
            	src1 = TranslateCast(T(INT), ty, src1);
				ty = T(INT);
			}
			GenerateBranch(ty, trueBB, JNZ, src1, NULL);
		}
		break;
	}
}
Symbol TranslateExpression(AstExpression expr)
{
	return (* ExprTrans[expr->op])(expr);
}