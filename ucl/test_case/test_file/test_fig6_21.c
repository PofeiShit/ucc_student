int num1, num2;
int *ptr1, *ptr2;
void f(void)
{
    int arr[4] = {10};
    ptr1 = &num1;
    ptr2 = &num2;
    *ptr1 = *ptr2;
}
