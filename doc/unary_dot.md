# unary dot
---
简单记录.运算符
```
struct test
{
    int a;
};
void main()
{
    struct test p;
    p.a = 1;
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
	leal -4(%ebp), %eax
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
在expr.c的ParsePostfixExpression()添加TK_DOT分支，
```
case TK_DOT:
    CREATE_AST_NODE(p, Expression);
    p->op = OP_MEMBER;
    p->kids[0] = expr;
    NEXT_TOKEN;
    p->val = TokenValue;
    NEXT_TOKEN;
    expr = p;
    break;
```
创建Expression节点，设置节点op为OP_MEMBER,可以确定.操作符后面是a,所以直接赋值给p->val='a'

## 语义分析
---
1.在exprchk.c的CheckPostfixExpression()添加OP_MEMBER分支，分支调用 CheckMemberAccess()进行语义检查，
在表达式 p.a 中，p 和 a 相当于是 运算符.的两个操作数，dt 对应语法树结点的类型应是记录类型
expr->kids[0] = CheckExpression(expr->kids[0]);对成员选择运算符的左操作数进行语义检查

2.结构体对象 dt 可当作左值，由此 dt.a 也可充当左值;但由于函数调用的返回值只存放在临时变量中，则 func().a 也不可充当左值。
因此，expr->lvalue = expr->kids[0]->lvalue;会根据运算符“.”的左操作数是否为左值来设置整个表达式 dt.a 或者 func().a 的 lvalue 属性。

3.LookupField()检查一下结构体 p 中是否有名字为 a 的域成员, 如果在结 构体的定义中找不到成员 a，则说明 dt.a 是非法的表达式

4.最后把查询得到的关于域成员 a 的类型信息，通过expr->val.p = fld存放到 dt.a 表达式对应的语法树结点上。 结构体 struct filed 描述了域成员的相关信息


## 中间代码生成
---
1.TranslatePostfixExpression添加OP_MEMBER分支,在分支中调用TranslateMemberAccess函数生成中间代码

2.addr = AddressOf(TranslateExpression(p));用于计算出p.a的结构体成员的基地址和常量偏移，在确定基地址和常量偏移后，就可以调用 Offset()函数

3.Offset函数：当 C 程序员访问数组元素或结构体成员时，参数 addr 是数组或结构体的基地址，参数 voff 是“可变偏移(variable offset)”。当访问结构体成员 p.a 时，由于结构体成员 p.a 在结构体对象是固定的，此时voff参数为NULL。 第 2 行的另一个参数 coff 代表“常量偏移(const offset)。CreateOffset()来为不存在可变偏移的结构体成员创建一个符号对象

4.CreateOffset():base 代表基地址，coff 代表“常量偏移”。p.a 在对象 p 中的偏移为 0，但 p.a 和 p 的类型不一样，因此我们需要为 p.a 创建一个新的符号对象,而不能使用和 dt 一样的符号对象,CALLOC(p);用于在堆空间中分配一个符号对象.设置该符号对象的类型为 SK_Offset，设置其类型，保存其基地址对应的符号对象，存放常量 偏移，用于生成形如“arr[8]”的符号名。

## 汇编代码生成
---