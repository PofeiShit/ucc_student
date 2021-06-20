#include "ucl.h"
#include "lex.h"
#include "keyword.h"

#define CURSOR      (Input.cursor)

typedef int (*Scanner)(void);
static Scanner        Scanners[256];

union value TokenValue;
#define IS_EOF(cur) (*(cur) == END_OF_FILE && ((cur) - Input.base) == Input.size)
static void SkipWhiteSpace(void)
{
	int ch;

	ch = *CURSOR;
	while (ch == '\t' || ch == '\v' || ch == '\f' || ch == ' ' ||
	       ch == '\r' || ch == '\n' || ch == '/'  || ch == '#')
	{
		switch (ch)
		{
		case '\n':
			++CURSOR;        
			break;

		default:
			CURSOR++;
			break;
		}
		ch = *CURSOR;
	}
}

#define SINGLE_CHAR_SCANNER(t) \
static int Scan##t(void)       \
{                              \
    CURSOR++;                  \
    return TK_##t;             \
}
SINGLE_CHAR_SCANNER(LBRACE)
SINGLE_CHAR_SCANNER(RBRACE)
SINGLE_CHAR_SCANNER(LPAREN)
SINGLE_CHAR_SCANNER(RPAREN)
SINGLE_CHAR_SCANNER(SEMICOLON)

static int ScanEOF(void)
{
	return TK_END;
}

// return keyword or TK_ID
static int FindKeyword(char *str, int len)
{
	struct keyword *p = NULL;
	//index is 0 when "__int64", see keyword.h	static struct keyword keywords_[]
	int index = 0;

	p = keywords[index];
	while (p->name)
	{
		if (p->len == len && strncmp(str, p->name, len) == 0)
			break;
		p++;
	}	
	return p->tok;
}

// parse string starting with char
static int ScanIdentifier(void)
{
	unsigned char *start = CURSOR;
	int tok;
	// letter(letter|digit)*
	CURSOR++;
	while (IsLetterOrDigit(*CURSOR))
	{
		CURSOR++;
	}

	tok = FindKeyword((char *)start, (int)(CURSOR - start));
	if (tok == TK_ID)
	{
		TokenValue.p = InternName((char *)start, (int)(CURSOR - start));
	}
	return tok;
}
static int ScanIntLiteral(unsigned char *start, int len, int base)
{
	unsigned char *p = start;
	unsigned char *end = start + len;
	unsigned int i[2] = {0, 0};
	int tok = TK_INTCONST;
	int d = 0;

	while (p != end) {
		if (base == 16) {
			// TODO: ADD hex scan
			;
		} else {
			d = *p - '0';
		}
		i[0] += d;
		p++;
	}
	TokenValue.i[1] = 0;
	TokenValue.i[0] = i[0];
	return tok;
}
static int ScanNumericLiteral(void)
{
	unsigned char *start = CURSOR;
	int base = 10;
	// now support decimal
	CURSOR++;
	while (IsDigit(*CURSOR))
	{
		CURSOR++;
	}
	if (base == 16 || (*CURSOR != '.' && *CURSOR != 'e' && *CURSOR != 'E')) {
		return ScanIntLiteral(start, (int)(CURSOR - start), base);
	}
}
static int ScanStringLiteral(void)	// "abc"  or L"abc"
{
	
	char tmp[512];
	char *cp = tmp;

	int len = 0;
	int maxlen = 512;
	int ch = 0;	
	String str;
	size_t n = 0;
	CALLOC(str);
	CURSOR++;			// skip "

	while (*CURSOR != '"')
	{
		if (*CURSOR == '\n' || IS_EOF(CURSOR))
			break;
		else{
            ch = *CURSOR;
            CURSOR++;
		}
		cp[len] = (char)ch;
		len++;
	}
	CURSOR++;		// skip "

	AppendSTR(str, tmp, len, 0);
	TokenValue.p = str;

	return TK_STRING;
}

int GetNextToken(void)
{
	int tok;

	// PrevCoord = TokenCoord;
	SkipWhiteSpace();
	// TokenCoord.line = LINE;	// line number in the *.i for C compiler
	// TokenCoord.col  = (int)(CURSOR - LINEHEAD + 1);
	// use function pointer table to avoid a large switch statement.
	tok = (*Scanners[*CURSOR])();
	return tok;
}
void SetupLexer(void)
{
	int i;
	for (i = 0; i < END_OF_FILE + 1; i++) {
		if (IsLetter(i)) {
			Scanners[i] = ScanIdentifier;
		}
		else if (IsDigit(i)) {
			Scanners[i] = ScanNumericLiteral;
		}

	}
	Scanners[END_OF_FILE] = ScanEOF;
	Scanners['"'] = ScanStringLiteral;
	Scanners['{'] = ScanLBRACE;
	Scanners['}'] = ScanRBRACE;
	Scanners['('] = ScanLPAREN;
	Scanners[')'] = ScanRPAREN;
	Scanners[';'] = ScanSEMICOLON;

}
