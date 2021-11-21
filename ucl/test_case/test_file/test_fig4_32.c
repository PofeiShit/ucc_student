int arr[3][4];
typedef int (*ArrPtr)[4];
ArrPtr ptr = &arr[0];
int * ptr1 = &arr[0][0]; 
int ** ptr2 = &ptr1;
int main(int argc,char * argv[])
{ 
    **arr = 1;
    **ptr = 2;
    **ptr2 = 3;
    return 0;
}
