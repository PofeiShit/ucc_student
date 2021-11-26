typedef struct
{
    int a;
    int b;
    int num[4];
} Data;
Data dt;
int arr[4];
int arr2[3][5];
int a, b, c, i;
int *ptr;
int main(int argc, char *argv[])
{
    Data dt1 = dt;
    Data dt2 = {1, 2, 3};
    c = a + b;
    ptr = &a;
    c = dt.a + dt.b;
    c = arr[0] + arr[1];
    arr2[i][2] = 30;
    dt.num[3] = 50;
    return 0;
}
