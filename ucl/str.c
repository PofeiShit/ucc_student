#include "ucl.h"
/**
 * Different with identifier, ucc doesn't maintain a string pool for string literal. i.e.
 * Even two string literals' character sequence is identical, there are seperate copy for
 * them in memory.
 * 
 * ucc can handle string with arbitrary length unless there is no memory. AppendSTR() 
 * appends the characters or wide characters in tmp to the string str and adds a terminating 
 * '\0' or L'\0'
 */
void AppendSTR(String str, char *tmp, int len, int wide)
{

	int i, size;
	char *p;
	int times = 1;

	// PRINT_DEBUG_INFO(("len = %d",len));
	size = str->len + len + 1;
	// always save wide char as int, no matter	whether sizeof(WCHAR) is 2 or 4

	p = HeapAllocate(&StringHeap, size * times);
	for (i = 0; i < str->len * times; ++i)
	{
		p[i] = str->chs[i];
	}
	 
	for (i = 0; i < len * times; ++i)
	{
		p[i+str->len * times] = tmp[i];
	}
	str->chs = p;
	str->len = size - 1;
	// str->wide = wide;
	// add 0 at the end of the string
	if (! wide)
	{
		str->chs[size - 1] = 0;
	}

}
char *InternName(char *id, int len)
{
	// TODO
	char *p = HeapAllocate(&StringHeap, len + 1);
	for (int i = 0; i < len; i++)
		p[i] = id[i];
	return p;
}
