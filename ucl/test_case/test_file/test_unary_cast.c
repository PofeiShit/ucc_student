extern void* malloc(int n);
unsigned short b;
unsigned char c;
unsigned int d;
void main()
{
    int a;
    (char)a;
    (char*)malloc(16);
    a = b;
    a = c;
    d = b;
    d = c;
    b = a;
    b = d;
    b = c;
}
