int a = 1;
int *p = &a;
int arr1[] = {1, 2, 3};
int arr2[2] = {4, 5};
int arr3[][2] = {{6, 7}, {8, 9}};
int arr4[2][2] = {{10, 11}, {12, 13}};

struct Data
{
    int a, b;
    int c;
} dt = {20, 30};
struct Data1
{
    struct {
        int a, b;
    };
    int c;
} dt1 = {{20}, 30};
int *ptr2 = arr4[2];
void main()
{
    int arr5[] = {1, 2, 3};
    int arr6[2] = {4, 5};
    int arr7[][2] = {{6, 7}, {8, 9}};
    int arr8[2][2] = {{10, 11}, {12, 13}};

    struct Data
    {
        int a;
        int c;
    } dt = {20, 30};
    int *ptr3 = arr4[2];
}
