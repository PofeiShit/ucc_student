const int a;
int i = 5, j = 6;
const int *p1 = &i;
int * const p2 = &j;
int add(const int a, const int b)
{
    return a + b;
}
void main()
{
    const int *p3 = &i;
    int *const p4 = &j;
    p1 = &j;
    *p2  = 10;
    p3 = &j;
    *p4 = 11;
    i = add(3, 4);
}
