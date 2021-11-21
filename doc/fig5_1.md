# basic block
```
int f(int n)
{
    if (n < 1) {
        return 1;
    } else {
        return n * f(n-1);
    }
}
int main(int argc,char * argv[])
{
    int i = 1;
    while (i <= 10) {
        printf("f(%d)= %d\n", i, f(i));
        i++;
    }
    return 0;
}
```

1.函数f，T(Function)->T(INT)

2.if语句，TranslateIfStatement 进入有else的代码块，整体逻辑
```
BBx:
    if (expr is False)  
       jmp FalseBB
TrueBB:
    xxxx
    jmp nextBB
FalseBB:
    xxxx
nextBB:
    xxx
```
2.1 所以调用Not函数处理n < 1表达式,把OP_LESS 改成OP_GREAT_EQUAL; 然后调用TranslateBranch(Not(), falseBB, trueBB). GenerateBranch生成OP_GREAT_EQAUL的中间代码，最后生成汇编:movl 20(%ebp), %eax; compl $1, %eax; jge FalseBB;

2.2 然后StartBBlock(trueBB); TranslateStatement(ifStmt->thenStmt) 生成return 1;的汇编代码 movl $1, %eax; jmp xxx

2.3 GenerateJump(nextBB)

2.4 StartBBlock(falseBB); TranslateStatement(ifStmt->elseStmt) 生成 return n * f(n - 1); returnStatement调用TransalteExpression翻译后面的表达式: TranslateBianryExpression，根节点是OP_MUL, 左子树n, 右子树OP_CALL。函数调用参数n - 1; movl 20(%ebp), %eax; subl $1, %eax; pushl %eax, call f; addl $4, %esp; n对应生成汇编代码 movl 20(%ebp), %ecx; imull %ecx, %eax; .jmp nextBB;
```
.BB0:
    cmpl $1, 20(%ebp)
    jge .BB3
.BB1: // StartBBlock(trueBB) 创建
    movl $0, %eax
    jmp .BB6
.BB2: // return 语句会创建Block
    jmp .BB5 // GenerateJump(nextBB)
.BB3: // StartBBlock(falseBB)
    movl 20(%ebp), %eax
    subl $1, %eax
    pushl %eax
    call f
    addl $4, %esp
    movl 20(%ebp), %ecx
    imull %eax, %ecx
    movl %ecx, %eax
    jmp .BB6
.BB4: // return 语句创建
.BB5: // nextBB 创建
.BB6: // 函数退出创建
```

3.main函数的while语句逻辑
```
goto ContinueBB
LoopBB:
    stmt
ContinueBB:
    if expr goto LoopBB
NextBB:
    ...
```
以及对应的代码:
```
static void TranslateWhileStatement(AstStatement stmt)
{
	AstLoopStatement whileStmt = AsLoop(stmt);
	whileStmt->loopBB = CreateBBlock();
	whileStmt->contBB = CreateBBlock();
	whileStmt->nextBB = CreateBBlock();

	GenerateJump(whileStmt->contBB);

	StartBBlock(whileStmt->loopBB);
	TranslateStatement(whileStmt->stmt);

	StartBBlock(whileStmt->contBB);
	TranslateBranch(whileStmt->expr, whileStmt->loopBB, whileStmt->nextBB);
	
	StartBBlock(whileStmt->nextBB);
}
```
按照上面的代码直接写出汇编
```
    int i = 1;
    while (i <= 10) {
        printf("f(%d)= %d\n", i, f(i));
        i++;
    }
```
i++; => i += 1; => i = i + 1;
```
.BB0:
    movl $1, -4(%ebp)
    jmp .BB2
.BB1: // loopBB
    leal .str0, %eax;
    pushl -4(%ebp)     // 参数i 对应函数f
    movl %eax, -8(%ebp) // 回写 .str0
    call f
    addl $4, %esp
    pushl %eax
    pushl -4(%ebp) // 参数i 对应函数printf
    pushl -8(%ebp)
    call printf
    addl $12, %esp
    movl -4(%ebp), %ecx // i 用于返回
    movl -4(%ebp), %edx // i 用于++迭代判断
    addl $1, %edx
    movl %edx, -4(%ebp) // 完成i++;
.BB2: // contBB
    cmpl $10, -4(%ebp)
    jle .BB1

.BB3: // nextBB

```



