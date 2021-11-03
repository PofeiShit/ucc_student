int a = 10, b = 20, c;
int main()
{
    static int d;
    static int e = 5;
    c = a | b;
    c = a & b;
    c = a << 2;
    c = a >> 2;
    c = a + b;
    c = a - b;
    c = a * b;
    c = a / b;
    c++;
    c--;
    c = a % b;
    c = -a;
    c = ~a;
    return 0;
}
