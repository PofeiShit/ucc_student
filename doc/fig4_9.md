# string
```
char buf[] = "123456";
char *ptr = "654321";
int main(int argc,char * argv[])
{
    char buf2[] = "abcdef";
    char * ptr2 = "fedcba";
    printf("Hello World.\n");
    return 0;
}
```

1.字符串初始化，在CheckInitializerInternal中数组初始化进行检查一并处理, 要求用于初始化数组的字符串应形如“abc”或{“abc”},但不可以是{“abc”, 123}.如果 C 程序员没有指定字符数组的大小，则计算出其数组元素的个数。如果 C 程序员指定的数组元素个数正好和字符串长度一样，我们就舍弃字符串末尾的’\0’字符。对于形如 char str2[3] =“123456” 的数组容量不够的情况，则舍弃多余的字符，同时给出一个警告。最后则创建一个 struct initData 对象，并插入到链表的末尾。EmitGlobals生成汇编 .globl buf; buf: .string "123456"

2.同样字符串初始化，ptr符号类型T(Pointer)->T(char)。CheckInitializerInternal中ty是T(Pointer),进入IsScalarType(ty)分支检查"654321"。CheckPrimaryExpression调用AddString生成符号.str0。在最终EmitString生成.str0 .string "654321" 对于符号ptr仍然在EmitGlobals中生成.globl ptr; ptr: .string "654321"

3.char buf2[] = "abcdef"; 局部字符串初始化, 符号buf2类型T(Array)->T(char)，在CheckInitializerInternal中得到大小，在TransalteCompoundStatement调用AddString生成符号.str3,EmitStirng生成.str3 .string "abcdef"。 GenerateMove(ty, dst, src); ty->categ == B 进入 EmitMoveBlock 调用 X86_MOVB生成 leal dst, %edi; leal src, %esi; movl $7, %ecx; rep movsb; dst的偏移-8(%ebp), src就是.str3 

4."fedcba"进入IsScalaraType(ty)分支。CheckPrimaryExpression调用AddString生成符号.str2。EmitString生成.str1 .string "fedcba"。TransalteCompoundStatement 把符号.str2加载到寄存器中，然后GenerateMove(ty, dst, src);放到*ptr2中
leal .str1, %eax; movl %eax, -12(%ebp);

5.printf("Hello World.\n"); 根节点OP_CALL,左子树printf，右子树OP_STR ("Hello World.\n")。语义检查，CheckPrimaryExpression 调用AddString生成符号.str2。EmitString生成.str2 .string "Hello World.012"; 把符号.str2加载到寄存器，然后pushl 参数入栈
```
leal .str2, %eax
pushl %eax
call printf
addl, $4, %esp
```
