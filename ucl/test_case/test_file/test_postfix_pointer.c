typedef struct
{
    int a;
    int b;
} Data;
Data dt;
Data *ptr = &dt;
struct test
{
    int a;
};
void main()
{
    struct test *p;
    p->a = 10;
    ptr->b = 3;
    dt.b = 5;
}
