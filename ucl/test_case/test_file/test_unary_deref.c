int *ptr1;
int **ptr2 = &ptr1;
int arr[4];
void f(){}
void main()
{
    int *a;
    *a;
    ptr1 = *&a;
    *(arr+3) = 1;
    *arr = 2;
    (*f)();
}
