extern void* malloc(int n);
char * str1;
void h(char *str)
{
    static char * str2;
    char * str3;
    str2 = str;
    str3 = (char*)malloc(16);
}
void main()
{
    str1 = "Hello World";
    h(str1);
}
