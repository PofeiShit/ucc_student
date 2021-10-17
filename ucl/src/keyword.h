struct keyword
{
	char *name;
	int len;
	int tok;
};
static struct keyword keywordsA[] =
{
	{"auto", 4, TK_AUTO},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsB[] =
{
	{"break", 5, TK_BREAK},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsC[] =
{
	{"case", 4, TK_CASE},
	{"char", 4, TK_CHAR},
	{"const", 5, TK_CONST},
	{"continue", 8, TK_CONTINUE},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsD[] =
{
	{"default", 7, TK_DEFAULT},
	{"do", 2, TK_DO},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsE[] =
{
	{"else", 4, TK_ELSE},
	{"extern", 6, TK_EXTERN},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsF[] =
{
	{"for", 3, TK_FOR},
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
	{"signed", 6, TK_SIGNED},
	{"sizeof", 6, TK_SIZEOF},
	{"static", 6, TK_STATIC},
	{"struct", 6, TK_STRUCT},
	{"switch", 6, TK_SWITCH},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsT[] =
{
	{"typedef", 7, TK_TYPEDEF},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsU[] =
{
	{"unsigned", 8, TK_UNSIGNED},
	{NULL, 	0, TK_ID},
};
static struct keyword keywordsV[] = 
{
	{"void",     4, TK_VOID},
	{"volatile", 8, TK_VOLATILE},
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
