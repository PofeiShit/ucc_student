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
		case '/':
			if (CURSOR[1] != '/' && CURSOR[1] != '*')
				return;
			break;
		default:
			CURSOR++;
			break;
		}
		ch = *CURSOR;
	}
}


static int ScanBar(void)
{
	CURSOR++;
	if (*CURSOR == '|')		// ||
	{
		CURSOR++;
		return TK_OR;
	}
	else if (*CURSOR == '=')	// |=
	{
		CURSOR++;
		return TK_BITOR_ASSIGN;
	}
	else	// |
	{
		return TK_BITOR;
	}
}

static int ScanAmpersand(void)
{
	CURSOR++;
	if (*CURSOR == '&')		// &&
	{
		CURSOR++;
		return TK_AND;
	}
	else if (*CURSOR == '=')	// &=
	{
		CURSOR++;
		return TK_BITAND_ASSIGN;
	}
	else		// &
	{
		return TK_BITAND;
	}
}

static int ScanCaret(void)	// ^	exclusive or
{
	CURSOR++;
	if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_BITXOR_ASSIGN;
	}
	else
	{
		return TK_BITXOR;
	}
}
static int ScanLess(void)
{
	CURSOR++;
	if (*CURSOR == '<')
	{
		CURSOR++;
		if (*CURSOR == '=')
		{
			CURSOR++;
			return TK_LSHIFT_ASSIGN;		// <<=
		}
		return TK_LSHIFT;		// <<
	}
	else if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_LESS_EQ;		// <=
	}
	else
	{
		return TK_LESS;			// <
	}
}

static int ScanGreat(void)
{
	CURSOR++;
	if (*CURSOR == '>')
	{
		CURSOR++;
		if (*CURSOR == '=')		// >>=
		{
			CURSOR++;
			return TK_RSHIFT_ASSIGN;
		}
		return TK_RSHIFT;		// >>
	}
	else if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_GREAT_EQ;			// >=
	}
	else
	{
		return TK_GREAT;		// >
	}
}
static int ScanPlus(void)
{
	CURSOR++;
	if (*CURSOR == '+')
	{
		CURSOR++;
		return -1;			// ++
	}
	else if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_ADD_ASSIGN;	// +=
	}
	else
	{
		return TK_ADD;			// +
	}
}

static int ScanMinus(void)
{
	CURSOR++;
	if (*CURSOR == '-')
	{
		CURSOR++;
		return -1;			// --
	}
	else if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_SUB_ASSIGN;	// -=
	}
	else if (*CURSOR == '>')
	{
		CURSOR++;
		return -1;		// ->
	}
	else
	{
		return TK_SUB;			// -
	}
}

static int ScanStar(void)
{
	CURSOR++;
	if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_MUL_ASSIGN;		// *=
	}
	else
	{
		return TK_MUL;				// *
	}
}

static int ScanSlash(void)
{
	CURSOR++;
	if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_DIV_ASSIGN;		//	 /=
	}
	else
	{
		return TK_DIV;				// 		/
	}
}

static int ScanPercent(void)
{
	CURSOR++;
	if (*CURSOR == '=')
	{
		CURSOR++;
		return TK_MOD_ASSIGN;		// %=
	}
	else
	{
		return TK_MOD;				// %
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
SINGLE_CHAR_SCANNER(COMMA)

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
	
	if (*str != '_')
		index = ToUpper(*str) - 'A';
	p = keywords[index];
	while (p->name)
	{
		if (p->len == len && strncmp(str, p->name, len) == 0)
			break;
		p++;
	}	
	return p->tok;
}

static int ScanCharLiteral(void)
{
	char ch = 0;
	CURSOR++;
	ch = *CURSOR;
	CURSOR++;
	CURSOR++;
	TokenValue.i[0] = ch;
	TokenValue.i[1] = 0;
	return TK_INTCONST;
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
	int carry0 = 0, carry1 = 0;

	while (p != end) {
		if (base == 16) {
			// TODO: ADD hex scan
			;
		} else {
			d = *p - '0';
		}
		switch(base) 
		{
			case 10:
			{
				unsigned int t1, t2;
				// number * 10 = number * 8 + number * 2 = (number << 3) + (number << 1)				
				t1 = i[0] << 3;
				t2 = i[0] << 1;
				i[0] = t1 + t2;															
			}
			break;
		}
		if (i[0] > UINT_MAX - d)	// for decimal, i[0] + d maybe greater than UINT_MAX
		{
			carry0 += i[0] - (UINT_MAX - d);
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
static int ScanExclamation(void)
{
	CURSOR++;
	if (*CURSOR == '=') {
		CURSOR++;
		return TK_UNEQUAL;
	} else {
		return -1;
	}
}
static int ScanEqual(void)
{
	CURSOR++;
	if (*CURSOR == '=') {
		CURSOR++;
		return TK_EQUAL; // ==
	} else {
		return TK_ASSIGN; // = 
	}
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
	Scanners['\''] = ScanCharLiteral;
	Scanners['"'] = ScanStringLiteral;
	Scanners['+']  = ScanPlus;
	Scanners['-']  = ScanMinus;
	Scanners['*']  = ScanStar;
	Scanners['/']  = ScanSlash;
	Scanners['%']  = ScanPercent;
	Scanners['<']  = ScanLess;
	Scanners['>']  = ScanGreat;
	Scanners['!']  = ScanExclamation;
	Scanners['|']  = ScanBar;
	Scanners['&']  = ScanAmpersand;
	Scanners['^']  = ScanCaret;
	Scanners['{'] = ScanLBRACE;
	Scanners['}'] = ScanRBRACE;
	Scanners['('] = ScanLPAREN;
	Scanners[')'] = ScanRPAREN;
	Scanners[';'] = ScanSEMICOLON;
	Scanners[','] = ScanCOMMA;
	Scanners['='] = ScanEqual;	

}

