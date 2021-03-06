#ifndef __UCC_H_
#define __UCC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include "input.h"
#include "error.h"
#include "alloc.h"
#include "vector.h"
#include "str.h"
#include "lex.h"
#include "type.h"
#include "symbol.h"
#define ALIGN(size, align) ((align == 0) ? size: ((size + align - 1) & (~(align - 1))))

#include "gen.h"

#define EMPTY_OBJECT_SIZE 1
extern Heap CurrentHeap;
extern FILE *ASMFile;
extern char *ExtName;
extern char *ASMFileName;
extern struct heap ProgramHeap;
extern struct heap FileHeap;
extern struct heap StringHeap;
extern int ErrorCount;
#endif
