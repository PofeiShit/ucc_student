extern int a;
extern void* malloc(int n);
void main()
{
    a = 1;
    malloc(16);
}

