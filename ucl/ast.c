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
	fprintf(stderr, "Do_Expect(%s %d,%d):", CurFileName, CurFileLineNo, tok);
}
/**
 * Check if current token is in a token set
 */
int CurrentTokenIn(int toks[])
{
	int *p = toks;

	while (*p)
	{
		if (CurrentToken == *p)
			return 1;
		p++;
	}

	return 0;
}

