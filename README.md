# ucc learning
该仓记录自己对ucc的学习心得, 省略公共子表达式，汇编代码的优化，数据类型float和double。主要学习下常用c代码是如何生成汇编代码的。一边阅读pdf文章，一边拆解代码，只运行相对应的代码，然后添加测试用例学习了大半年。学习编译器后，对函数调用，结构体，变量初始化，以及面试经常问的const和static等知其所以然。平时写代码或者看书，都会去想想会如何转成汇编。正如pdf作者说的攻克UCC编译器这座山头的意义在于“它能让我们获得攻克其他山头的能力”。ucc学习暂时搞一段落，开始学习操作系统。共勉!

## Enviroment
1. x86-64
2. gcc

## Build
编译c文件:
```
$ ./build.sh
$ ./outpout/ucl -o test.s test.c
```

跑测试用例:
```
$ ./build.sh -DBUILD_LLT=ON
$ ./output/ucl
```

## run
生成可执行文件:
```
./ run.sh
```


