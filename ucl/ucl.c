#include "ucl.h"
#include "ast.h"
#include "target.h"

Heap CurrentHeap;
HEAP(StringHeap);
HEAP(ProgramHeap);
HEAP(FileHeap);
FILE *ASMFile;
char *ExtName = ".s";
char *ASMFileName = NULL;
static void Initialize(void)
{
	CurrentHeap = &FileHeap;
	InitSymbolTable();
	ASMFile = NULL;
}
static void Finalize(void)
{
	FreeHeap(&FileHeap);
}
static void Compile(char *file)
{
    AstTranslationUnit transUnit;
	Initialize();

    // parse preprocessed C file, generate an abstract syntax tree
	transUnit = ParseTranslationUnit(file);

	CheckTranslationUnit(transUnit);
	// translate the abstract synatx tree into intermediate code
	Translate(transUnit);

	// emit assembly code from intermediate code.
	// The kernel function is EmitIRInst(inst).
	// for example, see function EmitAssign(IRInst inst) 
	EmitTranslationUnit(transUnit);
	Finalize();
}

static int ParseCommandLine(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; ++i) {
		if (strncmp(argv[i], "-o") == 0) {
			i++;
			ASMFileName = argv[i];
		}
		else {
			return i;
		}
	}
	return i;
}
/**
 * The compiler's main entry point. 
 * The compiler handles C files one by one.
 */
int main(int argc, char *argv[])
{
	int i;

	CurrentHeap = &ProgramHeap;
	argc--; argv++;
	
	i = ParseCommandLine(argc, argv);
	SetupRegisters();
	SetupLexer();
	// SetupTypeSystem();
	/**
	(1)	All the heap space are allocated from ProgramHeap before
		the following for-statement.
		command-line info	/ basic type info
	(2)
		When we call compile(), we first call Initialize() to set 
		CurrentHeap to FileHeap.
		At the end of compile(), finalize() is called to clear the FileHeap.
		That is ,we allocate heap memory from FileHeap when we 
		are compiling a C file; Clear it after the compilation is finished.
		And then , compile the next C file.
	 */
	for (; i < argc; ++i)
	{
		Compile(argv[i]);
	}

	//return (ErrorCount != 0);
	return 1;
}
