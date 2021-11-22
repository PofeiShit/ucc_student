# complicated logcial 
```
int a, b, c, d;
int main(int arg, char *argv[])
{
    if ((a && b) || c) {
        d = 1;
    } else {
        d = 2;
    }
    return 0;
}
```
1 int a, b, c, d;全局声明 .comm a, 4; .comm b, 4; .comm c, 4; .comm d, 4;

2 if语句，TranslateBranch调用Not函数处理将OP_OR转成OP_AND,然后左右子树调用Not处理，变成!c和!(a && b). !(a & b)转换成!a || !b。
最后TranslateBranch的第一个参数为 (!a || !b) && !c。第二个参数为falseBB d = 2, 第三个参数为TrueBB: d = 1 进入OP_AND分支,处理左右子树。函数内trueBB: d = 2, falseBB: d = 1
```
	case OP_AND:
		rtestBB = CreateBBlock();
		TranslateBranch(Not(expr->kids[0]), falseBB, rtestBB);
		StartBBlock(rtestBB);
		TranslateBranch(expr->kids[1], trueBB, falseBB);
```
2.1 调用TranslateBranch(Not(!a || !b), falseBB, rtestBB)，Not(!a || !b) 转换 (!a || !b)为 a && b。这里falseBB就是d = 1, rtestBB为!c; 也就是变成TranslateBranch((a && b), d = 1, !c),继续翻译,进入分支OP_AND，trueBB:d=1, falseBB:!c. 此时就是翻译该函数：
TranslateBranch(Not(a), !c, b) 和 TranslateBranch(b, d = 1, !c);
代码如下
```
.BB0:
    cmpl $0, a
    je .BB2
.BB1: // StartBBlock(rtestBB)
    cmpl $0, b
    jne .BB3 // trueBB
.BB2: // falseBB !c
    cmpl $0, c
    je .BB4 
.BB3: // trueBB
    movl $1, d
    jmp .BB5
.BB4: // falseBB
    movl $2, d
.BB5:
    movl $0, %eax
```

