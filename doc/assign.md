# assign
赋值是如何生成汇编的
```
void main()
{
    int a;
    a = 1;
}
```

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
	subl $4, %esp
.BB0:
	movl $1, -4(%ebp)
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
	声明和语句没有变动，主要在expr.c，在ParseExpression，先ParseAssignExpression最低优先级，然后在ParseAssignmentExpression里面在ParsePostfix

## 语义分析
---
	碰到a = 1的时候，从符号表查找a，返回对应的符号，检查相应的type即可

## 中间代码生成
---
	transexpr.c中TransAssignmentExpression, a返回符号dst, 1创建符号src，GenerateMove: MOV dst src

## 汇编代码生成
---
	x86.c的EmitMove对应中间代码的MOV指令，Move(X86_MOVI4, DST, SRC1), 需要注意的局部变量a在GetAccessName是通过%ebp偏置得到的

# example
```
int *f(void)
{
	static int number;
	return &number;
}
int main(int argc, char *argv[])
{
	*f() += 3;
	return 0;
}
```

1 *f() += 3 语法树如下:
```
			+=						+=
		   /  \						/\
		  *    3    => 			   *<-+
		 /						  /	   \
		op_call			      op_call   3
	    /  						/
	   f    				   f
```
汇编代码:
```
call f
movl (%eax), %ecx
addl $3, %ecx
movl %ecx, (%eax)
```

2 *f() += *f() + 3
```
		+=					+=
		/\					/ \
	*f()  +    => 		 *f()<-+
		 / \				  	\
	  *f()  3			   		 +
	  							/ \
	  						*f()   3
```
汇编代码:
```
subl $24, %esp // 
call f // 函数调用返回值符号类型SK_Temp，名字t0，占用4字节
movl (%eax), %ecx // 调用+=左侧的*f()函数产生的代码， 解引用，t1,占用4字节
movl %eax, -4(%ebp) // 保存f()返回的地址
movl %ecx, -8(%ebp) // 因为要第二次调用 *f()， 需要把%ecx寄存器的值回写到内存中，调用者负责eax, ecx, edx寄存器
call f // 函数调用返回值 t2
movl (%eax), %ecx  // 解引用 t3
addl $3, %ecx // *f() + 3 // addl 结果 t4
movl -8(%ebp), %edx // 把最左侧的*f()的结果从内存再加在到寄存器中，
addl %ecx, %edx // 计算 *f() + *f() + 3. addl 结果 t5
movl -4(%ebp), %ebx // 最后GenerateIndirectMove, t5赋值给t0。 先将t0对应的地
movl %edx, (%ebx) // 计算的结果放到*f()中
```
