extern int printf(const char *fmt, ...);
int a = 0xFFFFFFFF;
int b = 0;
int main()
{
    if((signed int)a > (signed int)b){
        printf("(signed int)a > (signed int)b \n");
    }
    if((unsigned int)a > (unsigned int)b){
        printf("(unsigned int)a > (unsigned int)b \n");
    }
    if((signed int)a > (unsigned int)b){
        printf("(signed int)a > (unsigned int)b \n");
    }
    if((unsigned int)a > (signed int)b){
        printf("(unsigned int)a > (signed int)b \n");
    }
    return 0;
}
