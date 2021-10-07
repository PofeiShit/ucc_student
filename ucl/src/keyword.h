struct keyword
{
	char *name;
	int len;
	int tok;
};
static struct keyword keywordsA[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsB[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsC[] =
{
	{"char", 4, TK_CHAR},
	{"const", 5, TK_CONST},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsD[] =
{
	{"do", 2, TK_DO},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsE[] =
{
	{"else", 4, TK_ELSE},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsF[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsG[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsH[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsI[] =
{
	{"if", 2, TK_IF},
	{"int", 3, TK_INT},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsJ[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsK[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsL[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsM[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsN[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsO[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsP[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsQ[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsR[] =
{
	{"return", 6, TK_RETURN},
	{NULL, 		0, TK_ID},
};
static struct keyword keywordsS[] =
{
	{"sizeof", 6, TK_SIZEOF},
	{"static", 6, TK_STATIC},
	{"struct", 6, TK_STRUCT},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsT[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsU[] =
{
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsV[] = 
{
	{"void",     4, TK_VOID},
	{NULL,       0, TK_ID}
};
static struct keyword keywordsW[] = 
{
	{"while", 5, TK_WHILE},
	{NULL, 0, TK_ID}
};
/**
	classify keywords by their first letter,
	to speed up comparing.
	see function FindKeyword()
 */
static struct keyword *keywords[] =
{
	keywordsA,
	keywordsB,
	keywordsC,
	keywordsD,
	keywordsE,

	keywordsF,
	keywordsG,
	keywordsH,
	keywordsI,
	keywordsJ,

	keywordsK,
	keywordsL,
	keywordsM,
	keywordsN,
	keywordsO,

	keywordsP,
	keywordsQ,
	keywordsR,
	keywordsS,
	keywordsT,

	keywordsU,
    keywordsV,
	keywordsW,
};
