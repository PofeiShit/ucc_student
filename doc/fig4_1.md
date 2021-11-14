# sematic check
```
struct Data{
    int num;
};
struct Data a;
int b;
char d;
int d;
int main(int argc,char * argv[])
{
    a+b;
    case 8:
        b++;
    return 0;
}
```

1.在构建类型系统时，编译器也会进行语义检查，第9行声明的 d 与之前声明的类型不相容，在第8行中 d 被声明为 char，而在第9行又被声明为 int。这个错误报error:Incompatible with previous definition

2.第12行 a 是个结构体对象，而 b 是个整数，按 C 语言的语义，表达式 a+b 是非法的。对表达式的语义检查工作是在exprchk.c中进行的，错误是在CheckAddOP发现的REPORT_OP_ERROR

3.第13行的 case 语句没有出现在 switch 语句，这个错误是在 stmtchk.c 的CheckCastStatement发现的,A case shall appear in a switch statement
