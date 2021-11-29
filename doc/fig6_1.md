# assemble code example
```
int a = 10;
int f(int num)
{
    int b;
    b = 1;
    printf("b = %d \n", b);
    return num + 2;
}
int main(int argc, char *argv[])
{
    a = f(30);
    return 0;
}
```
1 int a = 10;
```
.globl a;
a: .long 10
```
2 b = 1;
```
movl $1, -4(%ebp)
```
3 printf("b = %d \n", b);
```
.str0 .string "b = %d \n"

leal .str0, %eax
pushl -4(%ebp)
pushl %eax
call printf
addl $8, %esp
```
4 return num + 2;
```
movl 20(%ebp), %eax
addl $2, %eax
jmp xxx
```
5 a = f(30)
```
pushl $30
call f
addl $4, %esp
movl %eax, a
```
