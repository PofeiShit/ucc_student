struct keyword
{
	char *name;
	int len;
	int tok;
};

static struct keyword keywordsV[] = 
{
	{"void",     4, TK_VOID},
	{NULL,       0, TK_ID}
};

/**
	classify keywords by their first letter,
	to speed up comparing.
	see function FindKeyword()
 */
static struct keyword *keywords[] =
{
    keywordsV
};
