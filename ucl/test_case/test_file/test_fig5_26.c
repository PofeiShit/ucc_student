extern int printf(const char *fmt, ...);
int a,b;
int main(int argc, char *argv[])
{
    switch(a)
    {
        case 1:
            b = 10;
            break;
        case 2:
            b = 20;
            break;
        case 20000:
            b = 20000;
            break;
        case 50:
            b = 50;
            break;
    }
    b = 60;
    printf("main()\n");
    return 0;
}
