# register example
```
int a, b, c, d, s1, s2, s3;
void f(void)
{
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
    s1 = (a + b);
    s2 = (c + d);
}
void g(void)
{
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
}
```

没有实现公共子表达式的汇编代码:
1 int a, b, c, d, s1, s2, s3;
```
.comm a, 4
.comm b, 4
.comm c, 4
.comm d, 4
.comm s1, 4
.comm s2, 4
.comm s3, 4
```

2   
```
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
    s1 = (a + b);
    s2 = (c + d);
```
```
    movl a, %eax
	addl b, %eax
	movl %eax, s1
	movl c, %ecx
	addl d, %ecx
	movl %ecx, s2
	movl a, %edx
	addl b, %edx
	movl c, %ebx
	addl d, %ebx
	addl %ebx, %edx
	movl %edx, s3
	movl a, %esi
	addl b, %esi
	movl %esi, s1
	movl c, %edi
	addl d, %edi
	movl %edi, s2
```

3 
```
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
```
```
    movl a, %eax
	addl b, %eax
	movl %eax, s1
	movl c, %ecx
	addl d, %ecx
	movl %ecx, s2
	movl a, %edx
	addl b, %edx
	movl c, %ebx
	addl d, %ebx
	addl %ebx, %edx
	movl %edx, s3
``` 