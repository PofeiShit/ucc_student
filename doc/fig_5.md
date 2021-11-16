# array check
```
int arr[3][4];
typedef int (*ArrPtr)[4];
ArrPtr ptr = &arr[0];
int *ptr3 = &arr[1][2];
int *ptr1 = &arr[0][0];
int ** ptr2 = &ptr1;
int main(int argc, char *argv[])
{
    ptr[1][2] = 1;
    arr[1][2] = 2;
    arr[0][0] = 3;
    ptr2[0][0] = 5;
    return 0;
}
```
1.符号arr类型T(ArrayType-48)->T(ArrayType-16)->T(INT)，对应汇编: .comm arr, 48

2.符号ArrPtr语法树 NK_InitDeclarator->NK_ArrayDeclarator->NK_PointerDeclarator->NK_NameDeclarator. 类型T(Pointer)->T(ArrayType)->T(INT)

3.声明符号ptr类型为ArrPtr符号类型，右边&arr[0],转换成arr + 0语法树，EmitGlobals生成汇编 .globl ptr; ptr: .long arr;

4.int *ptr3 = &arr[1][2]; 符号ptr类型T(Pointer)->T(INT), 右边转成arr + 24语法树，.globl ptr3; ptr3: .long arr+24;

5.同上 .globl ptr1; ptr1: .long arr;

6.ptr2符号类型T(Pointer)->T(Pointer)->T(INT)。右边符号ptr1的类型T(Pointer)->T(INT);取地址类型相容，同样转成ptr1 + 0; .globl ptr2; ptr2: .long ptr1

7.ptr[1][2] = 1; 等号左边语法树 OP_INDEX_2->OP_INDEX_1->ptr。先获得ptr的类型T(Pointer)->T(arrayType)->T(INT), 然后得到OP_INDEX_1的类型T(array)->T(INT),大小为16，然后继续获得OP_INDEX_2的类型T(INT),大小为24，在中间代码生成中，ptr是指针，ptr[1]的类型是数组，ptr[1][2]的类型是T(INT)且是左值，最终的解引用放到TranslateAssignment中防止重复的汇编指令，最终得到汇编代码 movl ptr, %eax; addl $24, %eax; movl $1, (%eax);

8.arr[1][2] = 2; 同理 movl $2, arr+24。这里可以看到指针和数组在汇编上的差距，一个需要中间寄存器，一个不需要。因为arr[0][0]和arr[1][2]的定位则不需要在运行时进行“提领”操作，要访问的内存单元的首地址在编译时就可以计算出来。

9.同上 movl $0, arr;

10.ptr2[0][0] = 5; movl ptr2, %eax; movl (%eax), %ecx; movl $5, (%ecx); ptr2的类型是T(Pointer)->T(Pointer)->T(INT), ptr2[0]类型是T(Pointer)->T(INT), 需要装换下语法树
```
            []                      *
            /\                     / \
          []  0     ===>>>        +
          /\                     / \
       ptr2 0                   *   0
                               / 
                              +
                             / \
                          ptr2  0
```
然后中间代码生成一直调用Deref解引用。
```
movl ptr2, %eax
movl (%eax), %ecx
movl $5, (%ecx)
```