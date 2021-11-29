int a, b, c, d, s1, s2, s3;
void f(void)
{
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
    s1 = (a + b);
    s2 = (c + d);
}
void g(void)
{
    s1 = (a + b);
    s2 = (c + d);
    s3 = (a + b) + (c + d);
}
