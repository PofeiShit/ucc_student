# break

break语句是如何生成汇编的
```
int main()
{
    int a;
    for (a = 0; a < 5; a++) {
        break;
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
// initBB
// goto testBB
.BB0:
    movl $0, -4(%ebp)
    jmp .BB4
// loopBB
.BB1:
    jmp .BB5
.BB2:
// incrBB
.BB3:
    movl -4(%ebp), %eax
    addl $1, %eax
    movl %eax, -4(%ebp)
// testBB
// true goto loopBB
.BB4:
    cmpl $5, -4(ebp)
    jl .BB1
// nextBB:
.BB5:
// exitBB
.BB6:
	movl %ebp, %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret
```

## 词法分析
---
在keyword.h加入break关键词，token.h增加TK_BREAK。

## 语法分析
---
stmt.c：

.在ParseStatement添加TK_BREAK分支调用ParseBreakStatement函数解析break语句.

## 语义分析
---
stmtchk:

.在Stmtcheckers函数指针数组中添加CheckBreakStatement函数指针，对break语句进行语义检查。

.AstBreakStatement的target成员指向当前函数的breakable的栈顶(loop或者switch语句)

## 中间代码生成
---
break;后直接跳转到loop语句或者switch语句的nextBB
```
static void TranslateBreakStatement(AstStatement stmt)
{
	AstBreakStatement brkStmt = AsBreak(stmt);
	GenerateJump(AsLoop(brkStmt->target)->nextBB);
	StartBBlock(CreateBBlock());
}
```
## 汇编代码生成
---