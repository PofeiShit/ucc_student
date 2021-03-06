# conditional operator
记录下条件操作符生成汇编
```
int b, c;
void main()
{
    int a;
    a ? 3 : 2;
	a = 300 ? b : c;
}

```
汇编代码:
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
	subl $8, %esp // 8 局部变量a占4个字节， ？表达式的结果占4个字节,CreateTemp把临时变量添加到fyms->ilocal链表中，然后LayoutFrame计算offset
.BB0:
	cmpl $0, -4(%ebp)
	je .BB2
.BB1:
	movl $3, -8(%ebp)
	jmp .BB4
.BB2:
	movl $2, -8(%ebp)
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
1 ParseConditionalExpression: 调用ParseBinaryExpression时的参数 Prec[OP_OR]指明了“逻辑或||”运算符的优先级,创建了子表达式a对应的节点，op域为OP_QUESTION的节点，CREATE_AST_NODE(condExpr->kids[1], Expression);构建了op域为OP_COLON的结点，子表达式3由ParseExpression所创建，而子表达式2由ParseConditionalExpression()所创建
```
		?
	   / \
	  a   :
	     / \
		3   2
```

## 语义分析
---
先对第一个操作数a进行语义检查，然后递归的调用CheckExpression()函数对3，2进行语义检查，如果都为算术类型，整个条件表达式的类型为3和2的公共类型

## 中间代码生成
---
a ? 3 : 2
按a照逻辑如下:
```
BB0:
    if (a) goto trueBB
falseBB:
    t = 2
    goto nextBB
trueBB:
    t = 3
nextBB:
```
按照!a的逻辑
```
BB0:
    if (!a) goto falseBB
trueBB:
    t = 3
    goto nextBB
falseBB:
    t = 2
nextBB:
```
两种写法都可以采取

## 汇编代码生成
---
略

# example
```
int a, b, c, d;
int main(int argc, char *argv[])
{   
	(a ? b : c) = d;
	return 0;
}
```
(a ? b : c) 的?节点的lvalue=0，不是左值不能修改,所以在CheckAssignmentExpression中的CanModify返回0报Error


# example1
```
int a, b, c;
void main()
{
    c = a > 0 ? b + 2 : b + 3;
}
```
t0 : b + 2
t0 : b + 3
t0会在多个基本块中被赋值，当离开基本块的时候，需要会写临时变量
```
.BB0:
	cmpl $0, a
	jle .BBx
.BB1:
	movl b, %eax
	addl $2, %eax
	movl %eax, %eax // EmitMove
	movl %eax, -4(%ebp) 
	/* ModifyVar先把变量的needwb=0,然后把寄存器%eax之前保存的值回写,(当前变量标志为0所以在这里是不会回写的，然后在needwb=1表示这个变量也是需要回写的，如果紧接是"无条件跳转","有条件跳转","通过跳转表进行跳转","函数调用"等情况，需要在EmitJump()、EmitBranch()、EmitIndirectJump()和 EmitCall()函数中调用对应的函数回写寄存器的值到当前函数栈中（跳转过去后，对应的BBlock也是需要使用寄存器的）。这里，是在EmitJump调用ClearRegs回写了%eax
	*/
	jmp .BB3
.BB2:
	movl b, %eax
	addl $3, %eax
	movl %eax, %eax
	// 这里回写是因为当前Block已经结束了，统一会回写寄存器的值到当前函数栈中。
	movl %eax, -4(%ebp)
.BB3:
	movl -4(%ebp), %eax
	movl %eax, c
```
