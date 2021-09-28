# unary ~
---
简单记录~运算符
```
void main()
{
    int a;
    ~a;
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
	movl -4(%ebp), %eax
	notl %eax
.BB1:
	movl %ebp, %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret

```

## 语法分析
---
在expr.c添加ParseUnaryExpression()添加TK_COMP和TK_DEC,TK_INC共享分支

## 语义分析
---
在exprchk.c添加 CheckUnaryExpression()添加OP_COMP分支调用CheckExpression()检查变量a

## 中间代码生成
---
在transexpr.c的TranslateUnaryExpression函数调用Simplify翻译"~a",做一些编译时的代数简化，根据OPMap找到OP_NOT对应中间代码指令为BCOM

## 汇编代码生成
---
~运算符有专门的汇编指令notl对应，根据opcode.h知道BCOM对应的func是Assign。这里有点小问题:x86linux.tpl并没有X86_BCOMI4,X86_BCOMU4模块，只有X86_COMI4,X86_COMI4.所以将EmitAssign的EmitBCOMI4分支修改成EmitCOMI4; 
处理“DST: ~SRC1”的一元运算，不必为 SRC2 分配寄存器，AllocateReg(inst, 1)也会因为src1是SK_Variable条件并不分配寄存器
movl -4(%ebp), %eax
notl %eax