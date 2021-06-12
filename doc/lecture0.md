# lecture 0
以最快的方式实现如下代码:
```
void main()
{
    puts("hello world");
}
```
commit id:a076fdbfd112f8c030f3df72aaccd58078019ea8

## 一些简化
1. alloc.c, error.c, file.c, str.c, vector.c, output.c这些内存分配，错误提示，文件输入和输出直接用，不做修改。
2. 直接先抛弃ucc的类型系统和中间代码优化
3. 词法分析，语法分析，语义分析，中间代码和汇编代码生成不涉及的都抛弃

## 词法分析
1. token.h 定义TK_VOID(void), TK_ID(puts), TK_LPAREN((), TK_RPAREN()), TK_STRING("hello world"), TK_LBRACE({), TK_RBRACE(}), TK_SEMICOLON(;), TK_NEWLINE("\n"), TK_END("EOF"),只涉及代码相关的
2. keyword.h 只保留void关键词
3. lex.c：保留核心GetNextToken函数
```bash
int GetNextToken(void)
{
    int tok;
    SkipWhiteSpace();
    tok = (*Scanners[*CURSOR])();
    return tok;
}
```
```
Scanners:函数指针数组，数组每个元素为处理各种token的函数地址
SetupLexer函数初始化了Scanners数组，主要ScanStringLiteral和ScanIdentifier函数处理"hello world"和puts符号
```
## 语法分析
1. ast.h中的AST_NODE_COMMON先抛弃struct coord coord成员,以及其他的节点和hello world无关的都相应的抛弃了，只保留主干部分，下面就是语法分析完对应的语法树
```bash
#define AST_NODE_COMMON \
    int kind; \
    struct astNode *next;
type struct astNode
{
    AST_NODE_COMMON
} *AstNode;
```
2. 语法就只保留函数相关的声明,语句,表达式

![image](parse.jpg)
```
标注黄色颜色的就是例子语法分析后所在的节点，fsym成员在之后会使用，在这里AstFunction的成员fdec和dec指向同一个(感觉可以再抛弃一个)
```
## 语义分析
1. 这部分代码主要是declchk.c，stmtchk.c，exprechk.c以及symbol.c
2. 语义分析就是添加函数符号和字符串符合，其他的功能就都删除了
函数符号结构体的成员变量在中间代码会被赋值，可以看中间代码节的图
![image](semantic.jpg)

3. 蓝色是语义分析的结果，可以看到之前语法分析的表达式叶子节点的val成员是指向具体的字符串，在语义分析阶段，val成员指向符号，该符号保存字符串,使用Strings字符串符号表保存符号(语法树和符号表都会指向符号)
4. checkdeclarator把AstFunctionDeclarator的id成员赋值

## 中间代码生成
1. transtmt.c和transexpr.c生成表达式中间代码，函数TransFunction中用到了语法分析中的函数符号fsym中的entryBB和exitBB成员，中间再调用TranslatStatement(func->stmt)处理语句以及表达式
```bash
static void TranslateFunction(AstFunction func)
{
    BBlock bb;
    FSYM = func->sym; // main函数
    FSYM->entryBB = CreateBBlock();
    FSYM->exitBB = CreateBBlock();
    CurrentBB= FSYM->entryBB;
    TranslateStatement(func->stmt); // 会把中间代码append到CurrentBB中
    StartBBlock(Fsym->exitBB);
    bb = FSYM->entryBB;
    while (bb != NULL) {
        bb->sym = CreateLabel(); //生成label符号，符号中字符串就是BB0,BB1...
        bb = bb->next;
    }
}
```
2. 再TransExpression函数中，调用TranslateFunctionCall函数处理左子树(puts)，直接返回该符号，然后处理右子树(Hello World)，使用AddressOf生成取地址中间代码，生成形参符号.
```bash
AddressOf->TryAddValue->GenerateAssign(t:temp, op:ADDR, src1:hello world, src2:NULL);
```
```
之后再生成函数调用的中间代码GenerateFunctionCall(recv:NULL, faddr:puts, args:hello world);
```
![image](translate.jpg)
```
红色为中间代码生成部分生成的,CFG部分直接抛弃，optimize也抛弃，相对于UCC这里就多了BB1的label。PS:CALL对应的IRinst的opds[2]应该指向t0符号，在汇编代码生成的图已经补充上
IRinst双向链表用来保存每个BBlock的中间指令
```


## 汇编代码生成
1. 这部分主要代码emit.c，x86.c,x86linux.c。
emit.c入口：
```bash
void EmitTranslationUnit(AstTranslationUnit transUnit)
{
    ...
    BeginProgram();
    Segment(DATA);

    EmitStrings();
    Segment(CODE);
    EmitFunctions(transUnit);
    EndProgram();    
}
```
```
主要是EmitStrings生成.str0 .string "hello world"汇编代码和EmitFunctions生成main函数定义和puts函数调用相关代码
```
2. EmitStrings函数

    EmitStrings针对字符串符号表生成汇编代码

```bash
static void EmitStrings(void)
{
    Symbol p = Strings;
    String str;
    while (p) {
        DefineGlobal(p);
        str = p->val.p;
        int size = str->len + 1;
        Define(str, size);
        p = p->next;
    }
    PutString("\n");
}
```
    DefineGlobal函数完成symbol的aname成员赋值:".str0"(话说GetAccessName不是更应该命名成GetAsmName嘛，获取汇编代码名字)
    DefineString函数具体字符串:".string hello world"

3. EmitFunctions函数

    EmitFunctions调用EmitFunction函数使用语法树生成汇编代码，其中EmitBBlock生成puts("hello world")部分;
```bash
void EmitFunction(FunctionSymbol fsym)
{
    ...
    Export((Symbol)fsym); // 生成 .globl main 图中绿色背景
    DefineLable((Symbol)fsym); // 标签main:

    EmitPrologu(stksize); // 生成函数定义头部

    bb = fsym->entryBB;
    while (bb != NULL) {
        DefineLabel(bb->sym); // 标签.BB0: 和标签.BB1:
        EmitBBlock(bb); 
        bb = bb->next;
    }
    EmitEpilogue(stksize); // 生成函数定义尾部
    PutString("\n");
}
```

4. EmitBBlock函数
```
    EmitBBlock函数中inst指向第一条中间代码指令，根据opcode=ADDR，调用EmitAddress函数:先分配寄存器eax(也是一个符号),然后将X86_ADDR模板leal %1 %0中的1和0作为下标获取inst->opds相对应的符号0:t0， 1:hello world。t0的reg不为空所以%0替换成%eax, %1则直接为.str0
    所以最后生成汇编：leal .str0 %eax
```

```
static void EmitIRInst(IRInst inst)
{
    struct irinst instc = *inst;
    (*Emitter[inst->opcode])(&instc);
    return;
}
static void EmitBBlock(BBlock bb)
{
    IRInst inst = bb->insth.next;
    while (inst != &bb->insth) {
        EmitIRInst(inst);
        inst = inst->next;
    }
    ClearRegs();
}
```
```
    调用puts函数对应调用EmitCall函数来解析,PushArgument函数处理inst->opds[2]保存的符号Sym(t0), PutASMCode(X86_PUSH, &p); X86_PUSH对应模板为"pushl %0",使用t0符号对应的reg替换，生成pushl %eax
```
```
    接着PutASMCode(X86_CALL, inst->opds);对应模板为call %1替换成call puts，然后分配函数栈空间X86_REDUCEF(addl $4, %esp)
    DST为null没有返回值。所以puts("hello world")对应的汇编为:
.BB0:
    leal .str0, %eax
    pushl %eax
    call puts
    addl $4, %esp
```

![image](emit.jpg)





