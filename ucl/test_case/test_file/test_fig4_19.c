void f()
{
    int h(int, int);
    int a;
}
int main(int argc, char *argv[])
{
    printf("%d \n", h(3,4));
    return 0;
}
int h(int a, int b)
{
    return a + b;
}
