int *ptr;
void f(){}
void main()
{
    int a;
    &a;
    a = &(*ptr);
    a = &f;
}
