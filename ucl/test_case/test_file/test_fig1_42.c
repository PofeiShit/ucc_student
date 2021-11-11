typedef struct
{
    int arr1[10];
    int arr2[20];
} Data;
typedef void FUNC();
void f()
{
}
FUNC *func = &f;
Data dt1, dt2;
int main()
{
    dt1 = dt2;
    func();
    (*func)();
    f();
    return 0;
}
