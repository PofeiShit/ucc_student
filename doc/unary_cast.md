# unary cast
---
简单记录cast运算符
```
void main()
{
    int a;
    (char)a;
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
    movl %al, -8(%ebp)
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
在expr.c的ParseUnaryExpression()函数中TK_LPAREN分支需要判断紧跟在左括号的token是不是类型名，如果是类型名，就要按（type-name)unary-expression去分析，如果不是则词法分析器需要回溯到左括号处，BeginPeekToken()和EndPeekToken()的作用。
kids[0]节点对应ParseTypeName()用于分析类型名，kids[1]节点对应变量a

## 语义分析
---
OP_CAST分支调用CheckTypeCast函数，先调用CheckTypeName检查且返回type，然后Adjust对变量a进行调整。最后再调用Cast函数来进行类型转换。
Cast函数调用CastExpression()用于构建转型运算的语法树节点，op=OP_CAST，ty=转型后的类型，kids[0]=转型前的表达式。
把float类型强制转换到char类型时候，第一步执行float到int的转换，第二步再进行int到char的转换

## 中间代码生成
---
TranslateCast函数,如果转换后和转换前类型相同就跳过，转换前为int，转换后为char，opcode=TRUI1(TRUNCATE I1)，然后将转换后的表达式结果保存到临时变量，	
```
    dst = CreateTemp(ty);
    GenerateAssign(ty, dst, opcode, src, NULL);
```
## 汇编代码生成
---
EmitCast函数cast X86_TRUI1用于实现I4到I1的截断操作,我们面对的中间指令形如<TRUI1, DST, SRC1, NULL>, 如果SRC1的值已经在寄存器中，不妨设其为eax，则我们可在得到eax的低8位对应的寄存器al， 否则，我们在reg = GetByteReg();获取一个单字节寄存器，不妨设其为al，接下来Move(X86_MOVI4, X86Regs[reg->val.i[0]], SRC1);会把占4字节的操作数SRC1的值传送到寄存器eax中，由于eax寄存器的低8位就是寄存器al，因此通过Move(X86_MOVI1, DST, reg);把 al的值传送给目的操作数DST，最终可产生形如“movl i4, %eax;movb %al, i1;”的汇编代码

# 例子1
---
```
extern void* malloc(int n);
void main()
{
	(char*)malloc(16);
}
```
## 语法分析
---
创建AstTypeName节点，根节点为 () OP_CAST, 左子树 char*, 右子树为malloc(16).  ParseTypeName char* 创建节点,declaration specifers:char + declarator:* ParseUnaryExpressoin然后调用ParsePostfixExpression 创建右子树 malloc(16), 根节点(),OP_CALL, 左子树为malloc, 右子树为参数链表16

## 语义分析
---
CheckUnaryExpression 调用 CheckTypeCast, 然后调用 CheckTypeName 检查(char*), 生成T(Pointer)->T(Char)类型, 然后 CheckFunctionCall 获得符号malloc的类型T(Pointer)->T(VOID), 根节点()的类型T(Pointer)->T(VOID), 然后 Cast 将右节点
