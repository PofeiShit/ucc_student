int arr[3][4];
typedef int (*ArrPtr)[4];
ArrPtr ptr = &arr[0];
int *ptr3 = &arr[1][2];
int *ptr1 = &arr[0][0];
int ** ptr2 = &ptr1;
void main()
{
    int a[3][4];
    a[1][2] = 5;
    ptr[1][2] = 1;
    arr[1][2] = 2;
    arr[0][0] = 3;
    ptr2[0][0] = 5;
}
