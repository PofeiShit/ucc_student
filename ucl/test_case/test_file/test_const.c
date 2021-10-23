const int a;
int i = 5, j = 6;
const int *p1 = &i;
int * const p2 = &j;
void main()
{
    p1 = &j;
    *p2  = 10;
}
