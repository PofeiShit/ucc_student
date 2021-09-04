#ifndef __LEX_H_
#define __LEX_H_

enum token
{
	TK_BEGIN,
#define TOKEN(k, s) k,
#include "token.h"
#undef  TOKEN
};

union value
{
	int i[2];
	float f;
	double d;
	void *p;
};

#define IsDigit(c)         (c >= '0' && c <= '9')
#define IsLetter(c)        ((c >= 'a' && c <= 'z') || (c == '_') || (c >= 'A' && c <= 'Z'))
#define IsLetterOrDigit(c) (IsLetter(c) || IsDigit(c))
#define ToUpper(c) 		   (c & ~0x20)
#define HIGH_3BIT(v)       ((v) >> (8 * sizeof(int) - 3) & 0x07)
#define HIGH_1BIT(v)       ((v) >> (8 * sizeof(int) - 1) & 0x01)
int  GetNextToken(void);

extern union value TokenValue;
#endif
