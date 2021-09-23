# unary ++/--
---
之前把二元运算符简单过了一遍，这里开始单目运算符++和--。
```
void main()
{
    int a;
    !a;
}
```
未优化的汇编代码
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
	subl $8, %esp
.BB0:
	cmpl $0, -4(%ebp)
	je .BB2
.BB1:
	movl $0, -8(%ebp)
	jmp .BB3
.BB2:
	movl $1, -8(%ebp)
.BB3:
.BB4:
	movl %ebp, %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret

```

## 语法分析
---
在expr.c添加ParseUnaryExpression()添加TK_NOT和TK_DEC,TK_INC共享分支

## 语义分析
---
在exprchk.c添加 CheckUnaryExpression()添加OP_Not分支调用CheckExpression()检查变量a

## 中间代码生成
---
在transexpr.c中添加TranslateUnaryExpression()添加OP_Not分支调用TranslateBranchExpression生成分支代码