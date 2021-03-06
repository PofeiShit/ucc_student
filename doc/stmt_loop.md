# loop三部曲-while
---

while语句是如何生成汇编的
```
int main()
{
    int a;
    while (a) {
        a = 1;
    }
}
```
未优化的汇编代码:
```
# Code auto-generated by UCC

.data
.text
.globl	main

main:
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	movl %esp, %ebp
    subl -4, %ebp
.BB0:
    jmp .BB2
.BB1:
    movl $1, -4(%ebp)
.BB2:
    cmpl $1, -4(%ebp)
    jne .BB1
.BB3:
.BB4:
	movl %ebp, %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret
```

## 词法分析
---
在keyword.h加入while关键词，token.h增加TK_WHILE

## 语法分析
---
循环属于函数定义内部,在astFunction结构体增加Vector loops用于记录循环

stmt.c：

.在ParseStatement添加TK_WHILE分支调用ParseWhileStatement函数解析While语句.

. a对应whileStmt->expr节点。a = 1对应ifStmt->stmt节点。

## 语义分析
---
stmtchk:

.在Stmtcheckers函数指针数组中添加CheckWhileStatement函数指针，对while语句进行语义检查

.向量CURRENTF->loops用于存放while, do-while, for循环语句。我们可以把CURRENTF->loops向量看成一个栈，在语义检查时，当遇到while语句时，我们把while语句入栈，在完成对while语句的语义检查时把它从CURRENTF->while中出栈。宏PushStatement(v,stmt)用于把语句 stmt 入栈 v，宏 PopStatement(v) 则用于把栈 v 的栈顶元素出栈。


## 中间代码生成
---
while语句的翻译方案

```
goto ContinueBB
LoopBB:
    stmt
ContinueBB:
    if expr goto LoopBB
NextBB:
    ...
```
根据如上伪代码得到如下代码
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
## 汇编代码生成
---

# do-while
---

区别在于中间代码的翻译方案:
```
loopBB:
    stmt
continueBB:
    if (expr) goto loopBB
nextBB:
    ...
```

# for
---
for循环可以表示成for (expr1; expr2; expr3) stmt
中间代码的翻译方案
```
	initExpr
	goto testBB
loopBB:
	stmt
continueBB
	incrExpr
testBB:
	if (expr is NULL or expr is True) goto loopBB
nextBB:
	...
```
expr2如果为空的话，就直接跳到loopBB
