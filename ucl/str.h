
#ifndef __STR_H_
#define __STR_H_

typedef struct string
{
	char *chs;
	int len;
} *String;
char* InternName(char *id, int len);
void AppendSTR(String str, char *tmp, int len, int wide);


#endif
