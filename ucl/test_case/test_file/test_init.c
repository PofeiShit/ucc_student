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
}
