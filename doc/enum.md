# Enum
记录enum的汇编生成
```
enum Color
{
    RED, GREEN = 3, BLUE,
} color;
```
汇编代码
```
.comm color, 4
```

## 语法分析
---
1.enum属于type specifers. Color 属于tag， }符号前面的代码在parseDeclarationSpecifers 调用ParseEnumSpecifiers 解析声明说明符， enumSpcifiers 结构体包含 id 和 enumerator-list。RED 属于enumerator，因此解析完Color后，在循环调用ParseEnumerator解析RED,GREEN,BLUE。结构体Enumerator 包含id 和 expression（=3）

## 语义分析
---
CheckDeclarationSpecifiers 根据type Specifiers的kind调用CheckEnumSpecifiers 创建符号Color的类型T(EnumType)，并将其添加到符号表中，再循环检查Enumerator,符号RED,GREEN,BLUE添加到符号表中，其类型为T(EnumType),使用enum的时候，不用成员符号访问成员，直接赋值即可 color=RED。符号color的类型为T(EnumType),添加到Globals链表中，在EmitGlobal生成汇编代码