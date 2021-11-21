# boolean
```
int a , b, c;
int main(int argc,char * argv[])
{
    if (a + b) {
        c = 1;
    } else {
        c = 2;
    }
    if (a && b) {
        c = 3;
    } else {
        c = 4;
    }
    if (a & b) {
        c = 5;
    } else {
        c = 6;
    }
    c = a || b;
    return 0;
}
```

1 ifStmt带有else。
||在TranslateBranch对应的逻辑
```
    case OP_OR
		rtestBB = CreateBBlock();
		TranslateBranch(expr->kids[0], trueBB, rtestBB);
		StartBBlock(rtestBB);
		TranslateBranch(expr->kids[1], trueBB, falseBB);
```
// a + b
.BB0:
    movl a, %eax
    addl b, %eax
    cmpl $0, %eax
    je .BB2
.BB1:
    movl $1, c
    jmp .BB3
.BB2:
    movl $2, c
// a && b
.BB3: 	//	TranslateBranch(expr->kids[0], trueBB, rtestBB);
    cmpl $0, a
    je .BB6
.BB4: // rtestBB
    // 		TranslateBranch(expr->kids[1], trueBB, falseBB);
    cmpl $0, b
    je .BB6
.BB5:
    movl $3, c
    jmp .BB7
.BB6: // trueBB
    movl $4, c
// a & b
.BB7:
    movl a, %eax
    andl b, %eax
    cml $0, %eax
    je .BB9
.BB8:
    movl $5, c
    jmp .BB10
.BB9:
    movl $6, c
// c = a || b
.BB10:
    cmpl $0, a
    jne .BB13
.BB11: // rtestBB
    cmpl $0, b
    jne .BB13
.BB12: // falseBB
    movl $0, -12(%ebp)
    jmp .BB14
.BB13: // 	StartBBlock(trueBB);
    movl $1, -12(%ebp)
.BB14: // 	StartBBlock(nextBB);
    



// a || b 代码
```
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
```