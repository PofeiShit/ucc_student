# extern
主要记录下extern是如何从c生成汇编语言的

全局声明如下:

```
extern int a;
void main()
{
    a = 1;
}
```
生成未优化的代码:
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
.BB0:
	movl $1, a
.BB1:
	movl %ebp, %esp
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret
```

## 词法分析
---
在keyword.h加入extern关键词，token.h增加TK_EXTERN

## 语法分析
---
decl.c:

.extern 在语法分析阶段作为storage classes。

## 语义分析
---

## 中间代码生成
---

## 汇编代码生成
---
符号表虽然保存了变量a，但是extern声明的全局变量在其他c文件生成对应的汇编文件中,所以EmitGlobals添加判断,不在汇编文件生成.comm a,4代码

# extern函数
---
```
extern void malloc(int n);
```
## 语法分析
---
语法上整个属于GlobalDeclaration，而不是Function

## 语义检查
---
CheckGlobalDeclaration 得到符号malloc 的类型，然后添加到符号表即可