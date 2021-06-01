#include "ucl.h"
#include "ast.h"

int CurrentToken;
int CurFileLineNo;
const char *CurFileName;

void Do_Expect(int tok)
{
	if (CurrentToken == tok) {
		NEXT_TOKEN;
		return ;
	}
	fprintf(stderr, "(%s %d):", CurFileName, CurFileLineNo);
}
