extern int printf(const char *fmt, ...);
int f(int n)
{
    if (n < 1) {
        return 1;
    } else {
        return n * f(n-1);
    }
}
int main(int argc,char * argv[])
{ 
    int i = 1;
    while (i <= 10) {
        printf("f(%d)= %d\n",i,f(i));
        i++; 
    }
    return 0;
}
