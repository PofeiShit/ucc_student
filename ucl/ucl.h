#ifndef __UCC_H_
#define __UCC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "input.h"
#include "error.h"
#include "alloc.h"
#include "vector.h"
#include "str.h"
#include "lex.h"

#include "symbol.h"
#define ALIGN(size, align) ((align == 0) ? size: ((size + align - 1) & (~(align - 1))))

#include "gen.h"

extern Heap CurrentHeap;
extern FILE *ASMFile;
extern char *ExtName;
extern char *ASMFileName;
extern struct heap ProgramHeap;
extern struct heap FileHeap;
extern struct heap StringHeap;
#endif
