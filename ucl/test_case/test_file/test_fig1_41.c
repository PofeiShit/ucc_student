int *ptr;
typedef struct{
    int date;
    int month;
    int year;
} Data;
Data dt;
int main()
{
    int number[16] = {2015};
    ptr = &number[1];
    *ptr = 2016;
    dt.year = *ptr;
    return 0; 
}
