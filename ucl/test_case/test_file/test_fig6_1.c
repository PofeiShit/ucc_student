int a = 10;
int f(int num)
{
    int b;
    b = 1;
    printf("b = %d \n", b);
    return num + 2;
}
int main(int argc, char *argv[])
{
    a = f(30);
    return 0;
}
