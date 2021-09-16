# Struct
这次讲解struct 结构体是怎么生成汇编的，struct理解了，对应c++的class也就熟悉了，也就是多成员函数嘛，但是成员函数编译阶段还是当作普通函数，不会跟着成员变量一起在类中实例
```
struct test {
	char a;
	int b;
} t;
```
未经优化的汇编代码
```
# Code auto-generated by UCC

.data

.comm   t,8

.text
```

## 词法分析
---
.添加struct关键词

## 语法分析
---
.结构体的语法分析还是挺复杂的，而且暂时没有支持位域。结构体其实就是实现了一种类型和(int, char)是平齐的,就是如果c源码不加t变量，类似int;编译器也是支持的，只不过是在语法分析阶段就结束了。
.结构体既然是类型，也就是从ParseDeclarationSpecifiers进入分析，依据关键词struct进入ParseStructOrUnionSpecifier继续分析，最终返回的是StructSpecifiers, 结构体内部的int a; char b;就是普通的声明调用ParseStructDeclaration(),声明=specifiers和declarator,解析各自的部分即可。

## 语义分析
---
.本文的struct是global的，从ParseGlobalDeclaration进入，接着调用CheckStructOrUnionSpecifier。三步走:生成struct类型(RecordType), 第二步：StartRecord,为test创建符号，第三步，检查{}内的declaration.EndRecord
.CheckStructDeclaration: checkSpecifiers + checkDeclarator. checkDeclarator需要将变量a和b关联到test符号(AddField),

.重点讲解下EndRecord,计算各成员在结构体中的偏移信息offset，这些信息决定了结构体对象在内存中的布局。
        while (fld) {
            //计算当前成员的偏移值, 一开始rty->size = 0, fld->ty->align = 1(char), 4(int)
            /* 
            1. rty->size:0, fld->ty->align: 1 (char a)
            2. fld->offset = rty->size = 0;
            3. rty->size = 0 + 1 = 1;
            4. rty->size:1, fld->ty->align: 4 (int b)
            5. fld->offset = rty->size = 4;
            6. rty->size = rty->size + fld->ty->size = 8
            */
 			fld->offset = rty->size = ALIGN(rty->size, fld->ty->align);
 			rty->size = rty->size + fld->ty->size;		
 			if (fld->ty->align > rty->align) {
 				rty->align = fld->ty->align;
 			}
 			fld = fld->next;
 		}
        // 8, 4
        rty->size = ALIGN(rty->size, rty->align);

.global_declaration做完specifiers后，将t变量添加到符号链表Globals

## 中间代码生成
---
.略

## 汇编代码生成
---
.EmitGlobals生成.comm t,8

